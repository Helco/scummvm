/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "edna/console.h"
#include "edna/db.h"
#include "edna/edna.h"
#include "edna/game/game.h"
#include "edna/scriptcommand.h"
#include "edna/sprite/character.h"
#include "edna/sprite/text.h"

#include "gui/debugger.h"

using namespace Common;

namespace Edna {

Script::Script(Game &game, const GameTransition &transition) : _game(game) {
	if (transition._script != 0) {
		_isScriptRunning = true;
		_scriptId = transition._script;
		_scriptLine = transition._scriptLine;
	}
}

bool Script::isPerforming() {
	const bool isPlayerDone = _game.player().state() == Character::kWaiting;

	bool isNpcDone = true;
	Character *npc = _currentNpc == 0 ? nullptr : dynamic_cast<Character *>(_game.objectById(_currentNpc));
	if (npc == nullptr)
		_currentNpc = 0; ///< stop searching for a destroyed object
	else
		isNpcDone = npc->state() == Character::kWaiting;

	bool isSoundDone = false;
	if (_currentSound != Audio::SoundHandle()) {
		isSoundDone = !g_system->getMixer()->isSoundHandleActive(_currentSound);
	} else if (_currentSoundDuration > 0) {
		if (_currentSoundDuration > g_engine->getElapsed())
			_currentSoundDuration -= g_engine->getElapsed();
		else
			isSoundDone = true;
	} else
		isSoundDone = true;

	if (isPlayerDone && (isNpcDone || _parallelPerformance) && isSoundDone) {
		if (_soundText != nullptr)
			_soundText->toggle(false);
		_soundText = nullptr;
		_currentSound = {};
		_currentSoundDuration = 0;
		_isPerforming = false;

		// TODO: Check if this logic sill works for parallel performance by player
		if (_parallelPerformance && !_isPerforming)
			_parallelPerformance = false;
	}

	return _isPerforming;
}

void Script::runNew(ScriptId scriptId, uint32 firstLine) {
	assert(!_isScriptRunning && !_isPerforming);
	assert(firstLine > 0);
	assert(firstLine <= g_engine->db().script(scriptId).size());
	_scriptId = scriptId;
	_scriptLine = firstLine;
	_isScriptRunning = true;
	resume();
}

void Script::resume() {
	assert(_isScriptRunning && !_isPerforming);
	if (!_isScriptRunning)
		return;

	_isPerforming = true;
	auto &console = g_engine->console();
	ScriptId prevScriptId = _scriptId; // opScript can change the script, we have to handle this
	bool shouldKeepRunning = true;
	auto scriptLines = g_engine->db().script(_scriptId);
	while (shouldKeepRunning && _scriptLine <= scriptLines.size()) {
		const auto &command = scriptLines[_scriptLine - 1]._command;
		if (debugChannelSet(2, kDebugScript)) {
			debugN(">%10d %3d ", _scriptId, _scriptLine);
			command.debugPrintLog();
		}
		if (console.hasBreakpoint(_scriptId, _scriptLine)) {
			console.attach();
			console.onFrame(); // to start the GUI immediately
		}

		shouldKeepRunning = command._handler == nullptr ||
			(this->*command._handler)(command);

		if (prevScriptId != _scriptId) {
			prevScriptId = _scriptId;
			scriptLines = g_engine->db().script(_scriptId);
		} else
			_scriptLine++;
	}
	_isScriptRunning = _scriptLine <= scriptLines.size();
	if (!_isScriptRunning)
		debugC(2, kDebugScript, "Finished script %u at %u", _scriptId, _scriptLine);
}

void Script::stop() {
	if (!_isScriptRunning)
		return;
	_isScriptRunning = 0;
	_isPerforming = 0;
	_parallelPerformance = 0;
}

bool Script::opAchievement(const ScriptCommand &line) {
	warning("STUB script op: Achievement");
	return true;
}

bool Script::opIfActive(const ScriptCommand &line) {
	if (!g_engine->db().roomObject(line._args._ifActive)._active)
		return true;
	assert(line._thenLine != nullptr);
	const auto &thenLine = *line._thenLine;
	return thenLine._handler == nullptr || (this->*thenLine._handler)(thenLine);
}

bool Script::opIfItemActive(const ScriptCommand &line) {
	if (g_engine->db().item(line._args._ifItemActive)._inventoryPos == 0)
		return true;
	assert(line._thenLine != nullptr);
	const auto &thenLine = *line._thenLine;
	return thenLine._handler == nullptr || (this->*thenLine._handler)(thenLine);
}

String Script::speechPath() {
	return String::format("speech_%s/%u-%u.ogg", g_engine->language(), _scriptId, _scriptLine);
}

bool Script::opSay(const ScriptCommand &line) {
	_game.player().say(line._args._say._text, speechPath().c_str());
	return false;
}

bool Script::opThink(const ScriptCommand &line) {
	_game.player().think(line._args._say._text, speechPath().c_str());
	return false;
}

bool Script::opSayNpc(const ScriptCommand &line) {
	auto *npc = dynamic_cast<Character *>(_game.objectById(line._args._say._npc));
	if (npc == nullptr)
		warning("@ %10u %3u: Invalid NPC id: %u", _scriptId, _scriptLine, line._args._say._npc);
	else {
		_currentNpc = npc->id();
		npc->say(line._args._say._text, speechPath().c_str());
	}
	return npc == nullptr;
}

bool Script::opSaySound(const ScriptCommand &line) {
	String audioPath;
	uint32 duration = strlen(line._args._saySound._text);
	if (line._args._saySound._sound == nullptr) {
		audioPath = speechPath();
		duration *= 40; // TODO: Add SaySoundProblemData handling
	} else {
		audioPath = String::format("soundfx/%s.ogg", line._args._saySound._sound);
		duration *= duration < 15 ? 150 : 75;
	}
	_currentSound = g_engine->playSpeech(audioPath.c_str());
	if (_currentSound == Audio::SoundHandle())
		_currentSoundDuration = duration;

	if (*line._args._saySound._text) {
		Point pos = line._args._saySound._pos;
		if (pos == kInvalidPoint)
			pos = { (int16)(_game.player().basePosX()) , (int16)(_game.player().pos().y - 10) };
		_soundText = new Text(pos, FontKind::InactiveFont, line._args._saySound._text, (TextFlags)(kTextWrapLines | kTextMoveIntoScreen));
		_game.texts().add(_soundText, DisposeAfterUse::YES);
	}

	return false;
}

bool Script::opChoice(const ScriptCommand &line) {
	warning("STUB script op: Choice");
	return true;
}

bool Script::opToggleChoice(const ScriptCommand &line) {
	const auto &args = line._args._toggleChoice;
	g_engine->db().toggleChoice(args._set, args._line, args._active);
	return true;
}

bool Script::opChangeInteraction(const ScriptCommand &line) {
	const auto &args = line._args._changeInteraction;
	const auto object = g_engine->db().roomObject(args._object);
	g_engine->db().setRoomInteractionScript(object._toInteraction, args._action, args._newScript);
	return true;
}

bool Script::opChangeItemInteraction(const ScriptCommand &line) {
	const auto &args = line._args._changeItemInteraction;
	g_engine->db().setItemScript(args._item, args._action, args._newScript);
	return true;
}

bool Script::opChangeItemImage(const ScriptCommand &line) {
	const auto &args = line._args._changeItemImage;
	g_engine->db().setItemIcon(args._item, args._image);
	return true;
}

bool Script::opChangeRoomItemInteraction(const ScriptCommand &line) {
	const auto &args = line._args._changeRoomItemInteraction;
	g_engine->db().setRoomItemInteraction(args._item, args._object, args._newScript);
	return true;
}

bool Script::opChangeInvOBMScript(const ScriptCommand &line) {
	const auto &args = line._args._changeItemItemInteraction;
	g_engine->db().setItemInteraction(args._item1, args._item2, args._newScript);
	return true;
}

bool Script::opChangeChoiceScript(const ScriptCommand &line) {
	const auto &args = line._args._changeChoiceScript;
	g_engine->db().setChoiceScript(args._set, args._line, args._newScript);
	return true;
}

bool Script::opScript(const ScriptCommand &line) {
	_scriptId = line._args._script;
	_scriptLine = 1;
	return true;
}

bool Script::opExit(const ScriptCommand &line) {
	// TODO: Check validation error: In script 10090312 6: Referenced object 10090301 does not have an exit
	// originally this should throw a SQL exception, so we are fine to expect all objects?
	const auto dbObject = g_engine->db().roomObject(line._args._exit._object);
	const auto dbInteraction = g_engine->db().roomInteraction(dbObject._toInteraction);
	_game.triggerExit(dbInteraction._toExit, _scriptId, _scriptLine + 1);
	return false;
}

bool Script::opParamExit(const ScriptCommand &line) {
	auto &next = g_engine->next();
	next._room = line._args._paramExit._target;
	next._walkIn = line._args._paramExit._walkIn;
	next._walkInDir = line._args._paramExit._direction;
	next._script = _scriptId;
	next._scriptLine = _scriptLine + 1;
	return false;
}

bool Script::opFade(const ScriptCommand &line) {
	const auto &args = line._args._fade;
	_game.fade(args._isWhite ? 255 : 0, args._isFadeIn ? 0.0f : 1.0f, args._duration);
	_game.player().wait(args._duration);
	return false;
}

bool Script::opTempoMorph(const ScriptCommand &line) {
	_game.fade(255, 1.0f, 7000);
	_game.player().wait(7000);
	if (line._args._tempoMorphAlternateSound)
		g_engine->playSpeech((String("soundfx/h-tempo2_") + g_engine->language() + ".ogg").c_str());
	else
		g_engine->playSpeech("soundfx/h-tempom.ogg");
	return false;
}

bool Script::opAnimatePlayer(const ScriptCommand &line) {
	warning("STUB script op: AnimatePlayer");
	return true;
}

bool Script::opAnimatePlayerP(const ScriptCommand &line) {
	warning("STUB script op: AnimatePlayerP");
	return true;
}

bool Script::opAnimateNpc(const ScriptCommand &line) {
	warning("STUB script op: AnimateNpc");
	return true;
}

bool Script::opWalk(const ScriptCommand &line) {
	_game.player().pathWalkTo(line._args._walk._target);
	return false;
}

bool Script::opWalkNpc(const ScriptCommand &line) {
	auto *npc = dynamic_cast<Character *>(_game.objectById(line._args._walk._npc));
	if (npc == nullptr)
		warning("@ %10u %3u: Invalid NPC id: %u", _scriptId, _scriptLine, line._args._walk._npc);
	else {
		_currentNpc = npc->id();
		npc->pathWalkTo(line._args._walk._target);
	}
	return npc == nullptr;
}

bool Script::opWalkNpcP(const ScriptCommand &line) {
	if (opWalkNpc(line))
		return true;
	_parallelPerformance = true;
	return false;
}

bool Script::opFreeWalk(const ScriptCommand &line) {
	_game.player().freeWalkTo(line._args._walk._target);
	return false;
}

bool Script::opFreeWalkNpc(const ScriptCommand &line) {
	auto *npc = dynamic_cast<Character *>(_game.objectById(line._args._walk._npc));
	if (npc == nullptr)
		warning("@ %10u %3u: Invalid NPC id: %u", _scriptId, _scriptLine, line._args._walk._npc);
	else {
		_currentNpc = npc->id();
		npc->freeWalkTo(line._args._walk._target);
	}
	return npc == nullptr;
}

bool Script::opPutPlayer(const ScriptCommand &line) {
	_game.player().pos() = line._args._putTarget;
	return true;
}

bool Script::opWait(const ScriptCommand &line) {
	_game.player().wait(line._args._waitDuration / g_engine->console().waitDivider());
	return false;
}

bool Script::opItemActivate(const ScriptCommand &line) {
	warning("STUB script op: ItemActivate");
	return true;
}

bool Script::opItemActivateSound(const ScriptCommand &line) {
	warning("STUB script op: ItemActivateSound");
	return true;
}

bool Script::opItemDeactivate(const ScriptCommand &line) {
	warning("STUB script op: ItemDeactivate");
	return true;
}

bool Script::opActivate(const ScriptCommand &line) {
	toggleObject(line._args._toggleObject, true);
	return true;
}

bool Script::opDeactivate(const ScriptCommand &line) {
	toggleObject(line._args._toggleObject, false);
	return true;
}

void Script::toggleObject(RoomObjectId objectId, bool isActive) {
	auto sprite = _game.objectById(objectId);
	if (sprite == nullptr)
		g_engine->db().toggleRoomObject(objectId, isActive);
	else
		sprite->toggle(isActive);
}

bool Script::opToggleTimer(const ScriptCommand &line) {
	const auto &args = line._args._toggleTimer;
	g_engine->db().toggleTimer(args._timer, args._active);
	
	const auto room = g_engine->db().room(_game.roomId());
	if (room._timer == args._timer)
		_game.timer().toggle(args._active);
	return true;
}

bool Script::opAnimateObject(const ScriptCommand &line) {
	auto sprite = dynamic_cast<AnimatedSprite *>(_game.objectById(line._args._animateObject));
	if (sprite == nullptr)
		warning("Could not find animated object %u", line._args._animateObject);
	else {
		sprite->toggle(true);
		sprite->startAnimation();
	}
	return false; // only waits one frame
}

bool Script::opLookAt(const ScriptCommand &line) {
	_game.player().lookIn(line._args._lookAt._direction);
	return true;
}

bool Script::opNpcLookAt(const ScriptCommand &line) {
	auto *npc = dynamic_cast<Character *>(_game.objectById(line._args._lookAt._npcId));
	if (npc == nullptr)
		warning("@ %10u %3u: Invalid NPC id: %u", _scriptId, _scriptLine, line._args._lookAt._npcId);
	else {
		_currentNpc = npc->id();
		npc->lookIn(line._args._lookAt._direction);
	}
	return npc == nullptr;
}

}

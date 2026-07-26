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
#include "edna/sprite/player.h"

#include "gui/debugger.h"

using namespace Common;

namespace Edna {

Script::Script(Game &game) : _game(game) { }

bool Script::isPerforming() {
	Character *npc = _currentNpc == 0 ? nullptr : dynamic_cast<Character *>(_game.objectById(_currentNpc));
	if (npc == nullptr)
		_currentNpc = 0; ///< stop searching for a destroyed object

	const bool isNpcDone = npc == nullptr || npc->state() == Character::kWaiting;
	const bool isPlayerDone = _game.player().state() == Character::kWaiting;
	if (isPlayerDone && (isNpcDone || _parallelPerformance)) {
		// TODO: Check whether some sound is still playing
		_isPerforming = false;

		// TODO: Check if this logic sill works for parallel performance by player
		if (_parallelPerformance && !_isPerforming)
			_parallelPerformance = false;
	}

	return _isPerforming;
}

void Script::runNew(ScriptId scriptId, uint32 firstLine) {
	assert(!_isPerforming);
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

	// TODO: Exit handling

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
	warning("STUB script op: IfItemActive");
	return true;
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
	warning("STUB script op: SaySound");
	return true;
}

bool Script::opChoice(const ScriptCommand &line) {
	warning("STUB script op: Choice");
	return true;
}

bool Script::opToggleChoice(const ScriptCommand &line) {
	warning("STUB script op: ToggleChoice");
	return true;
}

bool Script::opChangeInteraction(const ScriptCommand &line) {
	warning("STUB script op: ChangeInteraction");
	return true;
}

bool Script::opChangeItemInteraction(const ScriptCommand &line) {
	warning("STUB script op: ChangeItemInteraction");
	return true;
}

bool Script::opChangeItemImage(const ScriptCommand &line) {
	warning("STUB script op: ChangeItemImage");
	return true;
}

bool Script::opChangeRoomItemInteraction(const ScriptCommand &line) {
	warning("STUB script op: ChangeRoomItemInteraction");
	return true;
}

bool Script::opChangeInvOBMScript(const ScriptCommand &line) {
	warning("STUB script op: ChangeInvOBMScript");
	return true;
}

bool Script::opChangeChoiceScript(const ScriptCommand &line) {
	warning("STUB script op: ChangeChoiceScript");
	return true;
}

bool Script::opScript(const ScriptCommand &line) {
	_scriptId = line._args._script;
	_scriptLine = 1;
	return true;
}

bool Script::opExit(const ScriptCommand &line) {
	warning("STUB script op: Exit");
	return true;
}

bool Script::opParamExit(const ScriptCommand &line) {
	warning("STUB script op: ParamExit");
	return true;
}

bool Script::opFade(const ScriptCommand &line) {
	const auto &args = line._args._fade;
	_game.fade(args._isWhite ? 255 : 0, args._isFadeIn ? 0.0f : 1.0f, args._duration);
	_game.player().wait(args._duration);
	return false;
}

bool Script::opTempoMorph(const ScriptCommand &line) {
	warning("STUB script op: TempoMorph");
	return true;
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
	warning("STUB script op: Walk");
	return true;
}

bool Script::opWalkNpc(const ScriptCommand &line) {
	warning("STUB script op: WalkNpc");
	return true;
}

bool Script::opWalkNpcP(const ScriptCommand &line) {
	warning("STUB script op: WalkNpcP");
	return true;
}

bool Script::opFreeWalk(const ScriptCommand &line) {
	warning("STUB script op: FreeWalk");
	return true;
}

bool Script::opFreeWalkNpc(const ScriptCommand &line) {
	warning("STUB script op: FreeWalkNpc");
	return true;
}

bool Script::opPutPlayer(const ScriptCommand &line) {
	warning("STUB script op: PutPlayer");
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
	if (sprite == nullptr) {
		auto dbObject = g_engine->db().roomObject(objectId);
		if (dbObject._active != isActive) {
			dbObject._active = isActive;
			// TODO: Store dbObject back
		}
	}
	else
		sprite->toggle(isActive);
}

bool Script::opToggleTimer(const ScriptCommand &line) {
	warning("STUB script op: ToggleTimer");
	return true;
}

bool Script::opAnimateObject(const ScriptCommand &line) {
	auto sprite = dynamic_cast<AnimatedSprite *>(_game.objectById(line._args._animateObject));
	if (sprite == nullptr)
		warning("Could not animated object %u", line._args._animateObject);
	else {
		sprite->toggle(true);
		sprite->startAnimation();
	}
	return false; // only waits one frame
}

bool Script::opLookAt(const ScriptCommand &line) {
	warning("STUB script op: LookAt");
	return true;
}

bool Script::opNpcLookAt(const ScriptCommand &line) {
	warning("STUB script op: NpcLookAt");
	return true;
}

}

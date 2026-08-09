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

#include "edna/scriptcommand.h"

#include "gui/debugger.h"

using namespace Common;

namespace Edna {

static char *splitAt(char *&line, char sep) {
	char *first = line;
	char *split = strchr(line, sep);
	if (split == nullptr)
		return nullptr;
	*split = '\0';
	line = split + 1;
	return first;
}

static const char *getCommand(char *&line) {
	return splitAt(line, '(');
}

static char *getArguments(char *&line) {
	return splitAt(line, ')');
}

static const char *getNextArgumentRaw(char *&params) {
	const char *value = splitAt(params, ',');
	if (value != nullptr)
		return value;
	value = params;
	params += strlen(params);
	return value;
}

static bool getNextArgument(char *&params, const char *&value) {
	if (!*params)
		return false;
	value = getNextArgumentRaw(params);
	return true;
}

static bool getNextArgument(char *&params, bool &value) {
	const char *valueStr = getNextArgumentRaw(params);
	if (valueStr != nullptr) {
		if (!scumm_stricmp(valueStr, "true")) {
			value = true;
			return true;
		}
		if (!scumm_stricmp(valueStr, "false")) {
			value = false;
			return true;
		}
	}
	return false;
}

static bool getNextArgument(char *&params, uint32 &value) {
	const char *valueStr = getNextArgumentRaw(params);
	if (valueStr == nullptr)
		return false;
	char *end = nullptr;
	unsigned long valueRaw = strtoul(valueStr, &end, 10);
	if (end == nullptr || *end != '\0' || valueRaw > UINT32_MAX)
		return false;
	value = (uint32)valueRaw;
	return true;
}

static bool getNextArgument(char *&params, int32 &value) {
	const char *valueStr = getNextArgumentRaw(params);
	if (valueStr == nullptr)
		return false;
	char *end = nullptr;
	long valueRaw = strtol(valueStr, &end, 10);
	if (end == nullptr || *end != '\0' || valueRaw > INT32_MAX || valueRaw < INT32_MIN)
		return false;
	value = (int32)valueRaw;
	return true;
}

static bool getNextArgument(char *&params, Point &value, bool allowChar = false) {
	if (allowChar && strlen(params) > 5 && !scumm_strnicmp(params, "char,", 5)) {
		params += 5;
		value = kInvalidPoint;
		return true;
	}

	int32 x, y;
	if (!getNextArgument(params, x) || !getNextArgument(params, y))
		return false;
	value = { (int16)x, (int16)y };
	return  true;
}

static bool getNextArgument(char *&params, Direction &value) {
	const char *valueStr = getNextArgumentRaw(params);
	return valueStr != nullptr && parseDirection(valueStr, value);
}

static bool getNextArgument(char *&params, PlayerAction &value) {
	const char *valueStr = getNextArgumentRaw(params);
	return valueStr != nullptr && parsePlayerAction(valueStr, value);
}

static bool getNextColor(char *&params, bool &isWhite) {
	const char *color = nullptr;
	if (!getNextArgument(params, color) || color == nullptr ||
		(scumm_stricmp(color, "black") && scumm_stricmp(color, "white")))
		return false;
	isWhite = color[0] == 'w';
	return true;
}

static ScriptCommand *getNextCommand(char *&line) {
	if (*line == ',')
		line++;
	return new ScriptCommand(line);
}

ScriptCommand::ScriptCommand() { }

ScriptCommand::ScriptCommand(char *line) : _fullLength(strlen(line)) {
	assert(line != nullptr);
	// two special cases that do not adhere to the usual command structure -_-
	if (!scumm_stricmp(line, "tempomorphen")) {
		_function = line;
		_handler = &Script::opTempoMorph;
		_args._tempoMorphAlternateSound = false;
		return;
	} else if (!scumm_stricmp(line, "tempomorph2")) {
		_function = line;
		_handler = &Script::opTempoMorph;
		_args._tempoMorphAlternateSound = true;
		return;
	}

	// the usual pattern
	_function = getCommand(line);
	char *params = getArguments(line);
	if (_function == nullptr || params == nullptr)
		return;
	else if (!scumm_stricmp(_function, "achievement")) {
		_handler = &Script::opAchievement;
		// TODO: parse achievement kind
	} else if (!scumm_stricmp(_function, "ifActive")) {
		if (getNextArgument(params, _args._ifActive) && !*params)
			_handler = &Script::opIfActive;
		_thenLine.reset(getNextCommand(line));
	} else if (!scumm_stricmp(_function, "ifItemActive")) {
		if (getNextArgument(params, _args._ifItemActive) && !*params)
			_handler = &Script::opIfItemActive;
		_thenLine.reset(getNextCommand(line));
	} else if (!scumm_stricmp(_function, "say")) {
		_handler = &Script::opSay;
		_args._say._text = params;
	} else if (!scumm_stricmp(_function, "think")) {
		_handler = &Script::opThink;
		_args._say._text = params;
	} else if (!scumm_stricmp(_function, "sayNsc")) {
		if (getNextArgument(params, _args._say._npc)) {
			_handler = &Script::opSayNpc;
			_args._say._text = params;
		}
	} else if (!scumm_stricmp(_function, "saySound")) {
		if (getNextArgument(params, _args._saySound._pos, true)) {
			_handler = &Script::opSaySound;
			_args._saySound._sound = nullptr;
			_args._saySound._text = params;
		}
	} else if (!scumm_stricmp(_function, "saySoundFile")) {
		if (getNextArgument(params, _args._saySound._pos, true) &&
			getNextArgument(params, _args._saySound._sound)) {
			_handler = &Script::opSaySound;
			_args._saySound._text = params;
		}
	} else if (!scumm_stricmp(_function, "choice")) {
		if (getNextArgument(params, _args._choiceSet) && !*params)
			_handler = &Script::opChoice;
	} else if (!scumm_stricmp(_function, "activateChoice")) {
		if (getNextArgument(params, _args._toggleChoice._set) &&
			getNextArgument(params, _args._toggleChoice._line) &&
			getNextArgument(params, _args._toggleChoice._active) &&
			!*params)
			_handler = &Script::opToggleChoice;
	} else if (!scumm_stricmp(_function, "changeROISkript")) {
		if (getNextArgument(params, _args._changeInteraction._object) &&
			getNextArgument(params, _args._changeInteraction._action) &&
			getNextArgument(params, _args._changeInteraction._newScript) &&
			!*params)
			_handler = &Script::opChangeInteraction;
	} else if (!scumm_stricmp(_function, "changeInvoSkript")) {
		if (getNextArgument(params, _args._changeItemInteraction._item) &&
			getNextArgument(params, _args._changeItemInteraction._action) &&
			getNextArgument(params, _args._changeItemInteraction._newScript) &&
			!*params)
			_handler = &Script::opChangeItemInteraction;
	} else if (!scumm_stricmp(_function, "changeInvoImg")) {
		if (getNextArgument(params, _args._changeItemImage._item) &&
			getNextArgument(params, _args._changeItemImage._image) &&
			!*params)
			_handler = &Script::opChangeItemImage;
	} else if (!scumm_stricmp(_function, "changeBMSkript")) {
		if (getNextArgument(params, _args._changeRoomItemInteraction._item) &&
			getNextArgument(params, _args._changeRoomItemInteraction._object) &&
			getNextArgument(params, _args._changeRoomItemInteraction._newScript) &&
			!*params)
			_handler = &Script::opChangeRoomItemInteraction;
	} else if (!scumm_stricmp(_function, "changeInvoBMSkript")) {
		if (getNextArgument(params, _args._changeItemItemInteraction._item1) &&
			getNextArgument(params, _args._changeItemItemInteraction._item2) &&
			getNextArgument(params, _args._changeItemItemInteraction._newScript) &&
			!*params)
			_handler = &Script::opChangeItemInteraction;
	} else if (!scumm_stricmp(_function, "changeCLSkript")) {
		if (getNextArgument(params, _args._changeChoiceScript._set) &&
			getNextArgument(params, _args._changeChoiceScript._line) &&
			getNextArgument(params, _args._changeChoiceScript._newScript) &&
			!*params)
			_handler = &Script::opChangeChoiceScript;
	} else if (!scumm_stricmp(_function, "skript")) {
		if (getNextArgument(params, _args._script) && !*params)
			_handler = &Script::opScript;
	} else if (!scumm_stricmp(_function, "exit")) {
		if (getNextArgument(params, _args._exit._object) &&
			getNextArgument(params, _args._exit._target) &&
			!*params)
			_handler = &Script::opExit;
	} else if (!scumm_stricmp(_function, "paramExit")) {
		if (getNextArgument(params, _args._paramExit._target) &&
			getNextArgument(params, _args._paramExit._walkIn) &&
			getNextArgument(params, _args._paramExit._direction) &&
			!*params)
			_handler = &Script::opParamExit;
	} else if (!scumm_stricmp(_function, "fadeOut")) {
		_args._fade._isFadeIn = false;
		if (getNextColor(params, _args._fade._isWhite) &&
			getNextArgument(params, _args._fade._duration) &&
			!*params)
			_handler = &Script::opFade;
	} else if (!scumm_stricmp(_function, "fadeIn")) {
		_args._fade._isFadeIn = true;
		if (getNextColor(params, _args._fade._isWhite) &&
			getNextArgument(params, _args._fade._duration) &&
			!*params)
			_handler = &Script::opFade;
	} else if (!scumm_stricmp(_function, "animateSC")) {
		_args._animateCharacter._npc = 0;
		if (getNextArgument(params, _args._animateCharacter._actionMode) &&
			getNextArgument(params, _args._animateCharacter._duration) &&
			!*params)
			_handler = &Script::opAnimatePlayer;
	} else if (!scumm_stricmp(_function, "animateSCP")) {
		_args._animateCharacter._npc = 0;
		if (getNextArgument(params, _args._animateCharacter._actionMode) &&
			getNextArgument(params, _args._animateCharacter._duration) &&
			!*params)
			_handler = &Script::opAnimatePlayerP;
	} else if (!scumm_stricmp(_function, "animateNsc")) {
		if (getNextArgument(params, _args._animateCharacter._npc) &&
			getNextArgument(params, _args._animateCharacter._actionMode) &&
			getNextArgument(params, _args._animateCharacter._duration) &&
			!*params)
			_handler = &Script::opAnimateNpc;
	} else if (!scumm_stricmp(_function, "walk")) {
		_args._walk._npc = 0;
		if (getNextArgument(params, _args._walk._target) && !*params)
			_handler = &Script::opWalk;
	} else if (!scumm_stricmp(_function, "freeWalk")) {
		_args._walk._npc = 0;
		if (getNextArgument(params, _args._walk._target) && !*params)
			_handler = &Script::opFreeWalk;
	} else if (!scumm_stricmp(_function, "walkNsc")) {
		if (getNextArgument(params, _args._walk._npc) &&
			getNextArgument(params, _args._walk._target) &&
			!*params)
			_handler = &Script::opWalkNpc;
	} else if (!scumm_stricmp(_function, "walkNscP")) {
		if (getNextArgument(params, _args._walk._npc) &&
			getNextArgument(params, _args._walk._target) &&
			!*params)
			_handler = &Script::opWalkNpcP;
	} else if (!scumm_stricmp(_function, "freeWalkNsc")) {
		if (getNextArgument(params, _args._walk._npc) &&
			getNextArgument(params, _args._walk._target) &&
			!*params)
			_handler = &Script::opFreeWalkNpc;
	} else if (!scumm_stricmp(_function, "putSC")) {
		if (getNextArgument(params, _args._putTarget) && !*params)
			_handler = &Script::opPutPlayer;
	} else if (!scumm_stricmp(_function, "wait")) {
		if (getNextArgument(params, _args._waitDuration) && !*params)
			_handler = &Script::opWait;
	} else if (!scumm_stricmp(_function, "itemActivate")) {
		if (getNextArgument(params, _args._toggleItem) && !*params)
			_handler = &Script::opItemActivate;
	} else if (!scumm_stricmp(_function, "itemActivateSound")) {
		if (getNextArgument(params, _args._toggleItem) && !*params)
			_handler = &Script::opItemActivateSound;
	} else if (!scumm_stricmp(_function, "itemDeactivate")) {
		if (getNextArgument(params, _args._toggleItem) && !*params)
			_handler = &Script::opItemDeactivate;
	} else if (!scumm_stricmp(_function, "activate")) {
		if (getNextArgument(params, _args._toggleObject) && !*params)
			_handler = &Script::opActivate;
	} else if (!scumm_stricmp(_function, "inactivate")) {
		if (getNextArgument(params, _args._toggleObject) && !*params)
			_handler = &Script::opDeactivate;
	} else if (!scumm_stricmp(_function, "activateTimer")) {
		if (getNextArgument(params, _args._toggleTimer._timer) &&
			getNextArgument(params, _args._toggleTimer._active) &&
			!*params)
			_handler = &Script::opToggleTimer;
	} else if (!scumm_stricmp(_function, "animate")) {
		if (getNextArgument(params, _args._animateObject) && !*params)
			_handler = &Script::opAnimateObject;
	} else if (!scumm_stricmp(_function, "lookAt")) {
		_args._lookAt._npcId = 0;
		if (getNextArgument(params, _args._lookAt._direction) && !*params)
			_handler = &Script::opLookAt;
	} else if (!scumm_stricmp(_function, "nscLookAt")) {
		if (getNextArgument(params, _args._lookAt._npcId) &&
			getNextArgument(params, _args._lookAt._direction) &&
			!*params)
			_handler = &Script::opNpcLookAt;
	} else
		_handler = nullptr;
}

template<typename T>
static void debugPrint(const ScriptCommand &cmd, T print) {
	enum {
		kStart = 0,
		kFirstArg,
		kArg
	} state = kStart;
	const char *const end = cmd._function + cmd._fullLength;
	const char *ptr = cmd._function - 1;
	while (ptr + 1 < end) {
		ptr++; // skipping the null-terminator before a segment

		if (*ptr == ',') { // this can only happen for conditions
			print("),");
			ptr++;
			state = kStart;
			continue;
		}
		if (state == kFirstArg) {
			state = kArg;
			print("(");
		} else if (state == kArg)
			print(",");
		else
			state = kFirstArg;
		print(ptr);
		ptr += strlen(ptr);
	}
	if (state != kFirstArg) {
		print(")\n");
		assert(ptr + 1 == end); // otherwise we overshot the end
	}
}

void ScriptCommand::debugPrintLog() const {
	debugPrint(*this, [](const char *s) { debugN(s); });
}

void ScriptCommand::debugPrintConsole() const {
	auto debugger = g_engine->getDebugger();
	assert(debugger != nullptr);
	debugPrint(*this, [debugger](const char *s) { debugger->debugPrintf(s); });
}


}

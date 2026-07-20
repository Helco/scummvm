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

#include "common/file.h"
#include "common/debug.h"
#include "edna/db.h"

using namespace Common;

namespace Edna {

// The validation is only triggered by the console command "validate"

uint32 DB::validate() {
	return
		validateScripts() +
		validateAnimations() +
		validateAnimationFrames() +
		validateCharAnimSets() +
		validateChoices() +
		validateItems() +
		validateItemInteractions() +
		validateRooms() +
		validateRoomObjects() +
		validateRoomObjectDisplays() +
		validateRoomInteractions() +
		validateRoomItemInteractions() +
		validateRoomExits() +
		validateTopics() +
		validateNPCs() +
		validateWalkableAreas() +
		validateTimers();
}

static uint32 validateScreenBounds(int32 x, int32 y, const char *sourceType, uint32 sourceKey) {
	if (x >= 0 && y >= 0 && x < kScreenWidth && y < kScreenHeight)
		return 0;
	debug("In %s %u: invalid screen bounds: %d %d", sourceType, sourceKey, x, y);
	return 1;
}

template<typename TValue>
uint32 DB::SimpleDataSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const {
	if (key == 0 || _map.contains(key))
		return 0;
	debug("In %s %u: invalid %s ref: %u", sourceType, sourceKey, _typeName, key);
	return 1;
}

template<typename TValue>
uint32 DB::TwoKeyDataSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const {
	const auto it = find_if(_map.begin(), _map.end(), [&](const TwoKeyMap<TValue>::Node &pair) {
		return pair._key.first == key;
	});
	if (it != _map.end())
		return 0;
	debug("In %s %u: invalid %s ref: %u", sourceType, sourceKey, _typeName, key);
	return 1;
}

template<typename TValue>
uint32 DB::SequenceSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const {
	if (key == 0 || _map.contains(key))
		return 0;
	debug("In %s %u: invalid %s ref: %u", sourceType, sourceKey, _typeName, key);
	return 1;
}

template<typename TValue>
uint32 DB::SequenceSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const {
	if (key == 0 || _map.contains(key))
		return 0;
	debug("In %s %u %u: invalid %s ref: %u", sourceType, sourceKey1, sourceKey2, _typeName, key);
	return 1;
}

template<typename TValue>
uint32 DB::SequenceSet<TValue>::validateRef(uint32 key1, uint32 key2, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const {
	auto lines = get(key1, false);
	auto it = find_if(lines.begin(), lines.end(), [key2](const TValue &v) { return v._line == key2; });
	if (it != lines.end())
		return 0;
	debug("In %s %u %u: invalid %s ref: %u %u", sourceType, sourceKey1, sourceKey2, _typeName, key1, key2);
	return 1;
}

uint32 DB::validateScripts() const {
	// a complete script validation would take much much more effort than this...
	uint32 errors = 0;
	for (const auto &line : _scripts._items) {
		if (line._command._function == nullptr || line._command._handler == nullptr) {
			debug("In script %u %u: invalid command", line._script, line._line);
			errors++;
		} else
			errors += validateScriptCommand(line);
	}
	for (const auto &script : _scripts._map) {
		auto firstLine = _scripts._items[script._value._begin]._line;
		if (firstLine != 1) {
			debug("In script %u: first line has line number %u", script._key, firstLine);
			errors++;
		}
	}
	return errors;
}

uint32 DB::validateCharAnimSets() const {
	uint32 errors = 0;
	for (const auto &pair : _charAnimSets._map) {
		errors += _animations.validateRef(pair._value._left, "character animation set (left)", pair._key.first);
		errors += _animations.validateRef(pair._value._right, "character animation set (right)", pair._key.first);
		errors += _animations.validateRef(pair._value._forward, "character animation set (forward)", pair._key.first);
		errors += _animations.validateRef(pair._value._back, "character animation set (back)", pair._key.first);
	}
	return errors;
}

uint32 DB::validateChoices() const {
	uint32 errors = 0;
	for (const auto &line : _choices._items) {
		if (strlen(line._text) == 0)
			debug("In choice %u %u: empty text", line._set, line._line);
		errors += _scripts.validateRef(line._script, "choice set", line._set);
	}
	return errors;
}

uint32 DB::validateRooms() const {
	uint32 errors = 0;
	for (const auto &pair : _rooms._map) {
		errors += validatePath(pair._value._background, "room", pair._key);
		errors += validateOptPath(pair._value._music, "room", pair._key, "", ".ogg");
		errors += _walkableAreas.validateRef(pair._value._walkAreaId, "room", pair._key);
		if (pair._value._vspeed < 0 || pair._value._hspeed < 0) {
			errors++;
			debug("In room %u: invalid speeds", pair._key);
		}
		errors += _charAnimSets.validateRef(pair._value._charAnimSet, "room", pair._key);
		errors += _timers.validateRef(pair._value._timer, "room", pair._key);
	}
	return errors;
}

uint32 DB::validateRoomObjects() const {
	uint32 errors = 0;
	for (const auto &pair : _roomObjects._map) {
		errors += _rooms.validateRef(pair._value._room, "room object", pair._key);
		if (pair._value._posX < 0 || pair._value._posY < 0 || pair._value._posZ < 0) {
			errors++;
			debug("In room object %u: invalid position", pair._key);
		}
		// room objects default to the empty "bildfolgen/leer/trans.png"
		errors += validatePath(pair._value._image, "room object", pair._key);

		// no need to check the associations, we check them at the source
	}
	return errors;
}

uint32 DB::validateRoomObjectDisplays() const {
	uint32 errors = 0;
	for (const auto &pair : _roomObjectDisplays._map) {
		errors += _roomObjects.validateRef(pair._value._object, "room object display", pair._key);
		errors += _animations.validateRef(pair._value._animation, "room object display", pair._key);
	}
	return errors;
}

uint32 DB::validateRoomInteractions() const {
	uint32 errors = 0;
	for (const auto &pair : _roomInteractions._map) {
		errors += _roomObjects.validateRef(pair._value._object, "room interaction", pair._key);
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In room interaction %u: empty name", pair._key);
		}
		errors += validateScreenBounds(pair._value._walkToX, pair._value._walkToY, "room interaction", pair._key);
		errors += _scripts.validateRef(pair._value._lookScript, "room interaction (look)", pair._key);
		errors += _scripts.validateRef(pair._value._useScript, "room interaction (use)", pair._key);
		errors += _scripts.validateRef(pair._value._takeScript, "room interaction (take)", pair._key);
		errors += _scripts.validateRef(pair._value._talkScript, "room interaction (talk)", pair._key);
	}
	return errors;
}

uint32 DB::validateRoomItemInteractions() const {
	uint32 errors = 0;
	for (const auto &pair : _roomItemInteractions._map) {
		errors += _scripts.validateRef(pair._value, "room item interaction", pair._key.first, pair._key.second);
	}
	return errors;
}

uint32 DB::validateRoomExits() const {
	uint32 errors = 0;
	for (const auto &pair : _roomExits._map) {
		errors += _roomInteractions.validateRef(pair._value._interaction, "room exit", pair._key);
		errors += _rooms.validateRef(pair._value._target, "room exit", pair._key);
		errors += validateScreenBounds(pair._value._walkToX, pair._value._walkToY, "room exit", pair._key);
	}
	return errors;
}

uint32 DB::validateItems() const {
	uint32 errors = 0;
	for (const auto &pair : _items._map) {
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In item %u: empty name", pair._key);
		}
		errors += validatePath(pair._value._icon, "item", pair._key, "", ".png");
		errors += validatePath(pair._value._icon, "item", pair._key, "", "_a.png");
		errors += _scripts.validateRef(pair._value._lookScript, "item (look)", pair._value._lookScript);
		errors += _scripts.validateRef(pair._value._useScript, "item (use)", pair._value._useScript);
		errors += _scripts.validateRef(pair._value._talkScript, "item (talk)", pair._value._talkScript);
	}
	return errors;
}

uint32 DB::validateItemInteractions() const {
	uint32 errors = 0;
	for (const auto &pair : _itemInteractions._map) {
		errors += _scripts.validateRef(pair._value, "item interaction", pair._key.first, pair._key.second);
	}
	return errors;
}

uint32 DB::validateTopics() const {
	uint32 errors = 0;
	for (const auto &pair : _topics._map) {
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In topic %u: empty name", pair._key);
		}
		errors += validatePath(pair._value._icon, "topic", pair._key);
		errors += _scripts.validateRef(pair._value._script, "topic", pair._key);
	}
	return errors;
}

uint32 DB::validateAnimations() const {
	uint32 errors = 0;
	for (const auto &pair : _animations._map) {
		errors += _animationFrames.validateRef(pair._key, "animation", pair._key);
	}
	return errors;
}

uint32 DB::validateAnimationFrames() const {
	uint32 errors = 0;
	for (const auto &frame : _animationFrames._items) {
		if (frame._altDuration < 1)
		{
			errors++;
			debug("In animation frame %u %u: invalid duration");
		}
		errors += _animations.validateRef(frame._animation, "animation frame", frame._animation);
		errors += validatePath(frame._image, "animation frame", frame._animation);
	}
	return errors;
}

uint32 DB::validateNPCs() const {
	uint32 errors = 0;
	for (const auto &pair : _npcs._map) {
		errors += _roomObjects.validateRef(pair._value._object, "npc", pair._key);
		errors += _charAnimSets.validateRef(pair._value._charAnimSet, "npc", pair._key);
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In npc %u: empty name", pair._key);
		}
		if (pair._value._vspeed < 0 || pair._value._hspeed < 0) {
			errors++;
			debug("In npc %u: invalid speeds", pair._key);
		}
	}
	return errors;
}

uint32 DB::validateWalkableAreas() const {
	uint32 errors = 0;
	for (const auto &pair : _walkableAreas._map) {
		errors += _rooms.validateRef(pair._value._room, "walkable area", pair._key);
		// there is an association redundancy, let's check they match
		Room room;
		if (_rooms._map.tryGetVal(pair._value._room, room) && room._walkAreaId != pair._key) {
			errors++;
			debug("In walkable area %u: room %u walkable area does not match (%u)", pair._key, room._id, room._walkAreaId);
		}
		errors += validatePath(pair._value._file, "walkable area", pair._key, "data/map/");
	}
	return errors;
}

uint32 DB::validateTimers() const {
	uint32 errors = 0;
	for (const auto &pair : _timers._map) {
		errors += _scripts.validateRef(pair._value._script, "timer", pair._key);
	}
	return errors;
}

uint32 DB::validateOptPath(const char *path, const char *sourceType, uint32 sourceKey, const char *basePath, const char *ext) {
	return *path ? validatePath(path, sourceType, sourceKey, basePath, ext) : 0;
}

uint32 DB::validatePath(const char *path, const char *sourceType, uint32 sourceKey, const char *basePath, const char *ext) {
	Path fullPath(basePath);
	fullPath.appendInPlace(path);
	fullPath.appendInPlace(ext);
	if (File::exists(fullPath))
		return 0;
	debug("In %s %u: missing file: %s", sourceType, sourceKey, fullPath.toString().c_str());
	return 1;
}

uint32 DB::validateScriptCommand(const ScriptLine &line) const {
	if (line._command._handler == nullptr || line._command._function == nullptr)
		return 0; // we already reported the parsing error, this method validates arguments

	const char *const function = line._command._function;
	const auto &args = line._command._args;
	// no validation for achievement
	if (!scumm_stricmp(function, "ifActive"))
		return _roomObjects.validateRef(args._ifActive, "script", line._script);
	if (!scumm_stricmp(function, "ifItemActive"))
		return _items.validateRef(args._ifItemActive, "script", line._script);
	if (!scumm_stricmp(function, "sayNsc"))
		return _npcs.validateRef(args._say._npc, "script", line._script);
	if (!scumm_stricmp(function, "saySoundFile"))
		return validateOptPath(args._saySound._sound, "script", line._script);
	if (!scumm_stricmp(function, "choice"))
		return _choices.validateRef(args._choiceSet, "script", line._script, line._line);
	if (!scumm_stricmp(function, "activateChoice"))
		return _choices.validateRef(args._toggleChoice._set, args._toggleChoice._line,
			"script", line._script, line._line);
	if (!scumm_stricmp(function, "changeROISkript")) {
		return _roomObjects.validateRef(args._changeInteraction._object, "script", line._script) +
			_scripts.validateRef(args._changeInteraction._newScript, "script", line._script);
	}
	if (!scumm_stricmp(function, "changeInvoSkript")) {
		return _items.validateRef(args._changeItemInteraction._item, "script", line._script) +
			_scripts.validateRef(args._changeInteraction._newScript, "script", line._script);
	}
	if (!scumm_stricmp(function, "changeInvoImg")) {
		return _items.validateRef(args._changeItemImage._item, "script", line._script) +
			validatePath(args._changeItemImage._image, "script", line._script);
	}
	if (!scumm_stricmp(function, "changeBMSkript")) {
		return _items.validateRef(args._changeRoomItemInteraction._item, "script", line._script) +
			_roomObjects.validateRef(args._changeRoomItemInteraction._object, "script", line._script) +
			_scripts.validateRef(args._changeRoomItemInteraction._newScript, "script", line._script);
	}
	if (!scumm_stricmp(function, "changeInvoBMSkript")) {
		return _items.validateRef(args._changeItemItemInteraction._item1, "script (first)", line._script) +
			_items.validateRef(args._changeItemItemInteraction._item2, "script", line._script) +
			_scripts.validateRef(args._changeItemItemInteraction._newScript, "script", line._script);
	}
	if (!scumm_stricmp(function, "changeCLSkript")) {
		return _choices.validateRef(args._changeChoiceScript._set, args._changeChoiceScript._line,
			"script", line._script, line._line) +
			_scripts.validateRef(args._changeChoiceScript._newScript, "script", line._script);
	}
	if (!scumm_stricmp(function, "skript"))
		return _scripts.validateRef(args._script, "script", line._script);
	if (!scumm_stricmp(function, "exit")) {
		return _roomObjects.validateRef(args._exit._object, "script", line._script) +
			_rooms.validateRef(args._exit._target, "script", line._script);
	}
	if (!scumm_stricmp(function, "paramExit"))
		return _rooms.validateRef(args._paramExit._target, "script", line._script);
	// no validation for fadeIn/fadeOut
	// no validation for animateSC(P) (we cannot check actionMode without knowing the charAnimSet at runtime)
	if (!scumm_stricmp(function, "animateNsc")) {
		if (_npcs.validateRef(args._animateCharacter._npc, "script", line._script))
			return 1;
		auto dbNpc = npc(args._animateCharacter._npc);
		auto charAnimSet = characterAnimationSet(dbNpc._charAnimSet, args._animateCharacter._actionMode, false);
		if (charAnimSet._id != dbNpc._charAnimSet) {
			debug("In script %u %u: Missing charAnimSet %u %u for animated NPC %u",
				line._script, line._line, dbNpc._charAnimSet, args._animateCharacter._actionMode, dbNpc._id);
			return 1;
		}
		return 0;
	}
	if (!scumm_stricmp(function, "walk"))
		return validateScreenBounds(args._walk._target.x, args._walk._target.y, "script", line._script);
	if (!scumm_stricmp(function, "walkNsc") || !scumm_stricmp(function, "walkNscP")) {
		return validateScreenBounds(args._walk._target.x, args._walk._target.y, "script", line._script) +
			_npcs.validateRef(args._walk._npc, "script", line._script);
	}
	// no validation for freeWalk(Nsc) (without path finding, the character is allowed to walk off-screen)
	// no validation for putSC (the script might freeWalk the player back in-screen)
	if (!scumm_stricmp(function, "itemActivate") ||
		!scumm_stricmp(function, "itemActivateSound") ||
		!scumm_stricmp(function, "itemDeactivate"))
		return _items.validateRef(args._toggleItem, "script", line._script);
	if (!scumm_stricmp(function, "activate") ||
		!scumm_stricmp(function, "inactivate"))
		return _roomObjects.validateRef(args._toggleObject, "script", line._script);
	if (!scumm_stricmp(function, "activateTimer"))
		return _timers.validateRef(args._toggleTimer._timer, "script", line._script);
	if (!scumm_stricmp(function, "animate")) {
		if (_roomObjects.validateRef(args._animateObject, "script", line._script))
			return 1;
		auto dbObject = roomObject(args._animateObject);
		auto dbDisplay = roomObjectDisplay(dbObject._toDisplay, false);
		if (dbDisplay._id != dbObject._toDisplay) {
			debug("In script %u %u: Missing display %u for animate object %u",
				line._script, line._line, dbObject._toDisplay, dbObject._id);
			return 1;
		}
		auto dbAnim = animation(dbDisplay._animation, false);
		if (dbAnim._id != dbDisplay._animation) {
			debug("In script %u %u: Missing animation %u for animtae object %u (display %u)",
				line._script, line._line, dbDisplay._animation, dbObject._id, dbDisplay._id);
			return 1;
		}
		return 0;
	}
	// no validation for lookAt
	if (!scumm_stricmp(function, "nscLookAt"))
		return _npcs.validateRef(args._lookAt._npcId, "script", line._script);
	return 0; // the function is unknown, this is a parser error, not a validation error
}

} // namespace Edna

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

template<typename TValue>
uint32 DB::SimpleDataSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const {
	if (key == 0 || _map.contains(key))
		return true;
	debug("In %s %u: invalid %s ref: %u", sourceType, sourceKey, _typeName, key);
	return false;
}

template<typename TValue>
uint32 DB::TwoKeyDataSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const {
	const auto it = find_if(_map.begin(), _map.end(), [&](const TwoKeyMap<TValue>::Node &pair) {
		return pair._key.first == sourceKey;
	});
	if (it != _map.end())
		return true;
	debug("In %s %u: invalid %s ref: %u", sourceType, sourceKey, _typeName, key);
	return false;
}

template<typename TValue>
uint32 DB::SequenceSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const {
	if (key == 0 || _map.contains(key))
		return true;
	debug("In %s %u: invalid %s ref: %u", sourceType, sourceKey, _typeName, key);
	return false;
}

template<typename TValue>
uint32 DB::SequenceSet<TValue>::validateRef(uint32 key, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const {
	if (key == 0 || _map.contains(key))
		return true;
	debug("In %s %u %u: invalid %s ref: %u", sourceType, sourceKey1, sourceKey2, _typeName, key);
	return false;
}

uint32 DB::validateScripts() const {
	// a complete script validation would take much much more effort than this...
	uint32 errors = true;
	for (const auto &line : _scripts._items) {
		if (strlen(line._command) < 3 ||
			!isAlpha(line._command[0]) ||
			strchr(line._command, '(') == nullptr ||
			strchr(line._command, ')') == nullptr) {
			debug("In script %u %u: invalid command", line._id, line._line);
			errors++;
		}
	}
	return errors;
}

uint32 DB::validateCharAnimSets() const {
	uint32 errors = true;
	for (const auto &pair : _charAnimSets._map) {
		errors += _animations.validateRef(pair._value._left, "character animation set (left)", pair._key.first);
		errors += _animations.validateRef(pair._value._right, "character animation set (right)", pair._key.first);
		errors += _animations.validateRef(pair._value._forward, "character animation set (forward)", pair._key.first);
		errors += _animations.validateRef(pair._value._back, "character animation set (back)", pair._key.first);
	}
	return errors;
}

uint32 DB::validateChoices() const {
	uint32 errors = true;
	for (const auto &line : _choices._items) {
		if (strlen(line._text) == 0)
			debug("In choice %u %u: empty text", line._id, line._line);
		errors += _scripts.validateRef(line._script, "choice set", line._id);
	}
	return errors;
}

uint32 DB::validateRooms() const {
	uint32 errors = true;
	for (const auto &pair : _rooms._map) {
		errors += validatePath(pair._value._background, "room", pair._key);
		errors += validateOptPath(pair._value._music, "room", pair._key);
		errors += _walkableAreas.validateRef(pair._value._walkAreaId, "room", pair._key);
		if (pair._value._vspeed < 0 || pair._value._hspeed < 0) {
			errors++;
			debug("In room %u: invalid speeds", pair._key);
		}
		// TODO: validate _guiId
		errors += _charAnimSets.validateRef(pair._value._charAnimSet, "room", pair._key);
		errors += _timers.validateRef(pair._value._timer, "room", pair._key);
	}
	return errors;
}

uint32 DB::validateRoomObjects() const {
	uint32 errors = true;
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
	uint32 errors = true;
	for (const auto &pair : _roomObjectDisplays._map) {
		errors += _roomObjects.validateRef(pair._value._object, "room object display", pair._key);
		errors += _animations.validateRef(pair._value._animation, "room object display", pair._key);
	}
	return errors;
}

uint32 DB::validateRoomInteractions() const {
	uint32 errors = true;
	for (const auto &pair : _roomInteractions._map) {
		errors += _roomObjects.validateRef(pair._value._object, "room interaction", pair._key);
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In room interaction %u: empty name", pair._key);
		}
		// TODO: Validate walkTo (probably 0,0->800/600) and defaultAction
		errors += _scripts.validateRef(pair._value._lookScript, "room interaction (look)", pair._key);
		errors += _scripts.validateRef(pair._value._useScript, "room interaction (use)", pair._key);
		errors += _scripts.validateRef(pair._value._takeScript, "room interaction (take)", pair._key);
		errors += _scripts.validateRef(pair._value._talkScript, "room interaction (talk)", pair._key);
	}
	return errors;
}

uint32 DB::validateRoomItemInteractions() const {
	uint32 errors = true;
	for (const auto &pair : _roomItemInteractions._map) {
		errors += _scripts.validateRef(pair._value, "room item interaction", pair._key.first, pair._key.second);
	}
	return errors;
}

uint32 DB::validateRoomExits() const {
	uint32 errors = true;
	for (const auto &pair : _roomExits._map) {
		errors += _roomInteractions.validateRef(pair._value._interaction, "room exit", pair._key);
		errors += _rooms.validateRef(pair._value._target, "room exit", pair._key);
		// TODO: Validate walkTo and lookDirection
	}
	return errors;
}

uint32 DB::validateItems() const {
	uint32 errors = true;
	for (const auto &pair : _items._map) {
		// TODO: Validate gui
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In item %u: empty name", pair._key);
		}
		errors += validatePath(pair._value._icon, "item", pair._key);
		// TODO: Validate defaultAction
		errors += _scripts.validateRef(pair._value._lookScript, "item (look)", pair._value._lookScript);
		errors += _scripts.validateRef(pair._value._useScript, "item (use)", pair._value._useScript);
		errors += _scripts.validateRef(pair._value._talkScript, "item (talk)", pair._value._talkScript);
	}
	return errors;
}

uint32 DB::validateItemInteractions() const {
	uint32 errors = true;
	for (const auto &pair : _itemInteractions._map) {
		errors += _scripts.validateRef(pair._value, "item interaction", pair._key.first, pair._key.second);
	}
	return errors;
}

uint32 DB::validateTopics() const {
	uint32 errors = true;
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
	uint32 errors = true;
	for (const auto &pair : _animations._map) {
		errors += _animationFrames.validateRef(pair._key, "animation", pair._key);
	}
	return errors;
}

uint32 DB::validateAnimationFrames() const {
	uint32 errors = true;
	for (const auto &frame : _animationFrames._items) {
		errors += _animations.validateRef(frame._id, "animation frame", frame._id);
		errors += validatePath(frame._image, "animation frame", frame._id);
	}
	return errors;
}

uint32 DB::validateNPCs() const {
	uint32 errors = true;
	for (const auto &pair : _npcs._map) {
		errors += _roomObjects.validateRef(pair._value._object, "npc", pair._key);
		errors += _charAnimSets.validateRef(pair._value._charAnimSet, "npc", pair._key);
		if (strlen(pair._value._name) == 0) {
			errors++;
			debug("In npc %u: empty name", pair._key);
		}
		// TODO: Validate font
		if (pair._value._vspeed < 0 || pair._value._hspeed < 0) {
			errors++;
			debug("In npc %u: invalid speeds", pair._key);
		}
	}
	return errors;
}

uint32 DB::validateWalkableAreas() const {
	uint32 errors = true;
	for (const auto &pair : _walkableAreas._map) {
		errors += _rooms.validateRef(pair._value._room, "walkable area", pair._key);
		// there is an association redundancy, let's check they match
		Room room;
		if (_rooms._map.tryGetVal(pair._value._room, room) && room._walkAreaId != pair._key) {
			errors++;
			debug("In walkable area %u: room %u walkable area does not match (%u)", pair._key, room._id, room._walkAreaId);
		}
		errors += validatePath(pair._value._file, "walkable area", pair._key);
	}
	return errors;
}

uint32 DB::validateTimers() const {
	uint32 errors = true;
	for (const auto &pair : _timers._map) {
		errors += _scripts.validateRef(pair._value._script, "timer", pair._key);
	}
	return errors;
}

uint32 DB::validateOptPath(const char *path, const char *sourceType, uint32 sourceKey) {
	return *path ? validatePath(path, sourceType, sourceKey) : 0;
}

uint32 DB::validatePath(const char *path, const char *sourceType, uint32 sourceKey, const char *ext) {
	return 0; // no archives are mapped yet
	/*uint32 errors;
	if (*ext)
		errors = File::exists(Path(String(path) + '.' + ext));
	else
		errors = File::exists(path);
	if (!errors)
		debug("In %s %u: missing file: %s", sourceType, sourceKey, path);
	return errors;*/
}

} // namespace Edna

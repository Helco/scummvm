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

#include "edna/db.h"

using namespace Common;

namespace Edna {

void DB::resetOverlay() {
	assert(_scripts._backup.empty());
	assert(_animations._overlay.empty());
	assert(_animationFrames._backup.empty());
	assert(_charAnimSets._overlay.empty());
	_choices.resetOverlay();
	_items.resetOverlay();
	_itemInteractions.resetOverlay();
	assert(_rooms._overlay.empty());
	_roomObjects.resetOverlay();
	assert(_roomObjectDisplays._overlay.empty());
	_roomInteractions.resetOverlay();
	_roomItemInteractions.resetOverlay();
	assert(_roomExits._overlay.empty());
	_topics.resetOverlay();
	assert(_npcs._overlay.empty());
	assert(_walkableAreas._overlay.empty());
	_timers.resetOverlay();
}

void DB::syncOverlay(Common::Serializer &s) {
    _choices.sync(s);
    _items.sync(s);
    _itemInteractions.sync(s);
    _roomObjects.sync(s);
    _roomInteractions.sync(s);
    _roomItemInteractions.sync(s);
    _topics.sync(s);
    _timers.sync(s);
}

template<class TValue>
void DB::SimpleDataSet<TValue>::overlay(const TValue &value) {
	assert(_map.contains(value._id));
	_overlay[value._id] = value;
}

template<class TValue>
void DB::SimpleDataSet<TValue>::resetOverlay() {
	_overlay.clear();
}

template<class TValue> 
void DB::SimpleDataSet<TValue>::sync(Serializer &s) {
    assert(_sync != nullptr);
    uint32 count = _overlay.size();
    s.syncAsUint32LE(count);

    if (s.isLoading()) {
        resetOverlay();
        for (uint32 i = 0; i < count; i++) {
            uint32 key = 0;
            s.syncAsUint32LE(key);
            TValue value = get(key, false);
            if (value._id != key)
                warning("Could not load %s id %u from save", _typeName, key);
            else {
                _sync(value, s);
                overlay(value);
            }
        }
    } else {
        for (auto &pair : _overlay) {
            uint32 key = pair._key;
            s.syncAsUint32LE(key);
            _sync(pair._value, s);
            assert(pair._key == pair._value._id);
        }
    }
}

template<class TValue>
void DB::TwoKeyDataSet<TValue>::overlay(uint32 key1, uint32 key2, const TValue &value) {
	assert(_map.contains({ key1, key2 }));
	_overlay[{ key1, key2 }] = value;
}

template<class TValue>
void DB::TwoKeyDataSet<TValue>::resetOverlay() {
	_overlay.clear();
}

template<class TValue>
void DB::TwoKeyDataSet<TValue>::sync(Serializer &s) {
    assert(_sync != nullptr);
    uint32 count = _overlay.size();
    s.syncAsUint32LE(count);

    if (s.isLoading()) {
        resetOverlay();
        for (uint32 i = 0; i < count; i++) {
			TwoKey key;
            s.syncAsUint32LE(key.first);
            s.syncAsUint32LE(key.second);

			TValue value;
			if (_map.tryGetVal(key, value)) {
				_sync(value, s);
				overlay(key.first, key.second, value);
			} else
                warning("Could not load %s id %u %u from save", _typeName, key.first, key.second);
        }
    } else {
        for (auto &pair : _overlay) {
            TwoKey key = pair._key;
            s.syncAsUint32LE(key.first);
            s.syncAsUint32LE(key.second);
            _sync(pair._value, s);
        }
    }
}

template<class TValue>
uint32 DB::SequenceSet<TValue>::getItemIndex(uint32 set, uint32 line) const {
	Range range;
	if (!_map.tryGetVal(set, range))
		return UINT32_MAX;

	const auto startIt = _items.begin() + range._begin;
	const auto endIt = startIt + range._count;
	auto it = find_if(startIt, endIt, [&](const TValue &other) { return other._line == line; });
	if (it == endIt)
		return UINT32_MAX;
	return (uint32)(it - _items.begin());
}

template<class TValue>
void DB::SequenceSet<TValue>::overlay(const TValue &value) {
	uint32 idx = getItemIndex(value._id, value._line);
	if (idx == UINT32_MAX)
		error("Invalid %s id %u %u for overlay", _typeName, value._id, value._line);

	if (!_backup.contains(idx))
		_backup[idx] = _items[idx];
	_items[idx] = value;

	assert(_backup[idx]._id == value._id && _backup[idx]._line == value._line);
}

template<class TValue>
void DB::SequenceSet<TValue>::resetOverlay() {
	for (const auto &pair : _backup)
		_items[pair._key] = pair._value;
	_backup.clear();
}

template<class TValue>
void DB::SequenceSet<TValue>::sync(Serializer &s) {
    assert(_sync != nullptr);
    uint32 count = _backup.size();
    s.syncAsUint32LE(count);

    if (s.isLoading()) {
        resetOverlay();
        for (uint32 i = 0; i < count; i++) {
            uint32 set = 0, line = 0;
            s.syncAsUint32LE(set);
            s.syncAsUint32LE(line);
            uint32 idx = getItemIndex(set, line);
            if (idx == UINT32_MAX)
                warning("Could not load %s id %u %u from save", _typeName, set, line);
            else {
                _backup[idx] = _items[idx];
                _sync(_items[idx], s);
            }
        }
    } else {
        for (auto &pair : _backup) {
            TValue value = _items[pair._key];
            s.syncAsUint32LE(value._id);
            s.syncAsUint32LE(value._line);
            _sync(value, s);
        }
    }
}

void DB::syncDBString(DBString &value, Serializer &s) {
	uint32 length = value._ownsString ? strlen(value.get()) : UINT32_MAX;
	s.syncAsUint32LE(length);

	// if the string is/was unmodified we do not save it
	if (length != UINT32_MAX) {
		if (s.isLoading()) {
			char *newString = (char *)malloc(length + 1);
			assert(newString != nullptr); // this fixes a MSVC warning
			newString[length] = '\0';
			s.syncBytes((byte *)newString, length);
			value = DBString::ownerOf(newString);
		}
		if (s.isSaving()) // double cast needed for compiler warning
			s.syncBytes((byte *)(const byte *)value.get(), length);
	}
}

void DB::syncChoice(Choice &value, Serializer &s) {
	s.syncAsByte(value._active);
	s.syncAsUint32LE(value._script);
}

void DB::syncItem(Item &value, Serializer &s) {
	syncDBString(value._icon, s);
	s.syncAsUint32LE(value._inventoryPos);
	s.syncAsUint32LE(value._lookScript);
	s.syncAsUint32LE(value._useScript);
	s.syncAsUint32LE(value._talkScript);
}

void DB::syncScriptId(ScriptId &value, Serializer &s) {
	s.syncAsUint32LE(value);
}

void DB::syncRoomObject(RoomObject &value, Serializer &s) {
	s.syncAsSint16LE(value._pos.x);
	s.syncAsSint16LE(value._pos.y);
	s.syncAsByte(value._active);
}

void DB::syncRoomInteraction(RoomInteraction &value, Serializer &s) {
	s.syncAsUint32LE(value._lookScript);
	s.syncAsUint32LE(value._useScript);
	s.syncAsUint32LE(value._takeScript);
	s.syncAsUint32LE(value._talkScript);
}

void DB::syncTopic(Topic &value, Serializer &s) {
	s.syncAsUint32LE(value._inventoryPos);
	s.syncAsUint32LE(value._topicRowPos);
}

void DB::syncTimer(Timer &value, Serializer &s) {
	s.syncAsByte(value._active);
}

}

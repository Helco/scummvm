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

#include "db.h"

using namespace Common;

namespace Edna {

/* The database in Edna is originally HSQLDB with the data stored in CSV files
 * We use the files in-place, adding zero-terminators where necessary.
 * A quirk is that lines may stretch across multiple lines, only the last
 * column is finished by a newline. 
 */

static char *loadFile(FileData &data, const Path &basePath, const char *fileName) {
	Path path = basePath.appendComponent(fileName);
	File file;
	if (!file.open(path)) {
		error("Could not open data file: %s", path.toString().c_str());
		return nullptr;
	}
	data->allocate((uint32)(file.size() + 1));
	if (file.read(data->data(), data->size() - 1) != data->size() - 1) {
		error("Could not read data file: %s", path.toString().c_str());
		return nullptr;
	}
	data[data->size() - 1] = '\0';
	return data->data();
}

static void skipWhitespace(char *&full) {
	while (*full && isSpace(*full))
		full++;
}

// the returned span is also null-terminated
static Span<char> nextCell(char *&full, bool isLastColumn = false) {
	char *const cell = full;
	uint32 length = 0;
	bool containsNewLine = false;
	while (*full) {
		if (isLastColumn) {
			if (*full == '\n') {
				if (length > 0 && full[-1] == '\r') {
					length--;
					full[-1] = '\0';
				}
				*full++ = '\0';
				skipWhitespace(full); // prepare for next line
				break;
			}
		} else {
			if (*full == ';') {
				*full++ = '\0';
				break;
			} else if (*full == '\n')
				containsNewLine = true;
		}
		length++;
		full++;
	}
	if (!containsNewLine)
		return Span<char>(cell, length);

	// if there are newlines there might be carriage returns.
	// those have to be removed
	char *cursor = cell;
	char *cr = strchr(cursor, '\r');
	while (cr != nullptr) {
		length--;
		memmove(cr, cr + 1, cell + length - cr);
		cursor = cr;
		cr = strchr(cursor, '\r');
	}
	return Span<char>(cell, length);
}

static char *nextString(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	return cell.data();
}

static uint32 nextInteger(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	char *end = nullptr;
	auto value = strtoul(cell.data(), &end, 10);
	if (end == nullptr || end == cell.data()) // we cannot check for '\0' as some lines have extra, unused cells...
		error("Could not extract integer from data file");
	return (uint32)value;
}

static float nextFloat(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	char *end = nullptr;
	float value = strtof(cell.data(), &end);
	if (end == nullptr || *end != '\0')
		error("Could not extract float from data file");
	return value;
}

static bool nextBool(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	if (strcmp(cell.data(), "true"))
		return true;
	if (strcmp(cell.data(), "false"))
		return false;
	error("Could not extract bool from data file");
}

DB::DB(const Path &path) : path(path) {
	loadScripts();
	loadCharAnimSets();
	loadChoices();
	loadRooms();
}

DB::~DB() {}

template<class TValue>
TValue DB::SimpleDataSet<TValue>::get(uint32 key, const char *name) const {
	TValue value;
	if (!_map.tryGetVal(key, value))
		error("Missing %s: %u", name, key);
	return value;
}

template<class TValue>
TValue DB::TwoKeyDataSet<TValue>::get(uint32 key1, uint32 key2, const char *name) const {
	TValue value;
	if (!_map.tryGetVal({ key1, key2 }, value ))
		error("Missing %s: %u %u", name, key1, key2);
	return value;
}

template<class TValue>
Span<const TValue> DB::SequenceSet<TValue>::get(uint32 key, const char *name) const {
	Range range;
	if (!_map.tryGetVal(key, range))
		error("Missing %s: %u", name, key);
	assert(range._begin < _items.size() && range._begin + range._count <= _items.size());
	return { &_items[range._begin], range._count };
}

template<class TValue>
template<class StrictWeakOrdering>
void DB::SequenceSet<TValue>::setupSequences(StrictWeakOrdering comp) {
	sort(_items.begin(), _items.end(), comp);

	uint32 begin = 0;
	for (uint32 i = 1; i < _items.size(); i++) {
		if (_items[begin]._id != _items[i]._id) {
			_map[_items[begin]._id] = { begin, i - begin };
			begin = i;
		}
	}
	_map[_items[begin]._id] = { begin, _items.size() - begin };
}

Span<const DB::ScriptLine> DB::script(ScriptId scriptId) const {
	return _scripts.get(scriptId, "script");
}

void DB::loadScripts() {
	char *full = loadFile(_scripts._data, path, "skript.csv");
	skipWhitespace(full);
	while (*full) {
		ScriptLine scriptLine;
		scriptLine._id = nextInteger(full);
		scriptLine._line = nextInteger(full);
		scriptLine._command = nextString(full);
		scriptLine._comment = nextString(full, true);
		_scripts._items.push_back(scriptLine);
	}

	_scripts.setupSequences([&](const ScriptLine &a, const ScriptLine &b) {
		return a._id != b._id
			? a._id < b._id
			: a._line < b._line;
	});
}

DB::CharacterAnimationSet DB::characterAnimationSet(CharAnimSetId setId, ActionModeId actionModeId) const {
	return _charAnimSets.get(setId, actionModeId, "character animation set");
}

void DB::loadCharAnimSets() {
	char *full = loadFile(_charAnimSets._data, path, "characteranimationset.csv");
	skipWhitespace(full);
	while (*full) {
		CharacterAnimationSet set;
		set._id = nextInteger(full);
		set._actionMode = nextInteger(full);
		set._name = nextString(full);
		set._left = nextInteger(full);
		set._right = nextInteger(full);
		set._forward = nextInteger(full);
		set._back = nextInteger(full, true);
		_charAnimSets._map.setVal({ set._id, set._actionMode }, set);
	}
}

Span<const DB::Choice> DB::choices(ChoiceSetId choiceId) const {
	return _choices.get(choiceId, "choice set");
}

void DB::loadChoices() {
	char *full = loadFile(_choices._data, path, "choiceliste.csv");
	skipWhitespace(full);
	while (*full) {
		Choice choice;
		choice._id = nextInteger(full);
		choice._line = nextInteger(full);
		choice._active = nextBool(full);
		choice._text = nextString(full);
		choice._script = nextInteger(full, true);
		_choices._items.push_back(choice);
	}

	_scripts.setupSequences([&](const ScriptLine &a, const ScriptLine &b) {
		return a._id != b._id
			? a._id < b._id
			: a._line < b._line;
	});
}

DB::Room DB::room(RoomId id) const {
	return _rooms.get(id, "room");
}

void DB::loadRooms() {
	char *full = loadFile(_rooms._data, path, "raum.csv");
	skipWhitespace(full);
	while (*full) {
		Room room;
		room._id = nextInteger(full);
		room._name = nextString(full);
		room._background = nextString(full);
		room._music = nextString(full);
		room._walkAreaId = nextInteger(full);
		room._vspeed = nextFloat(full);
		room._hspeed = nextFloat(full);
		room._baseYAtZeroScale = nextFloat(full);
		room._baseYAtFullScale = nextFloat(full);
		room._guiId = nextInteger(full);
		room._charAnimSet = nextInteger(full);
		room._timer = nextInteger(full, true);
		_rooms._map.setVal(room._id, room);
	}
}

}

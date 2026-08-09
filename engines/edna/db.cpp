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
#include "edna/db.h"

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
static StringSpan nextCell(char *&full, bool isLastColumn = false) {
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
		return StringSpan(cell, length);

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
	return StringSpan(cell, length);
}

static char *nextString(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	return cell.data();
}

static uint32 nextUint(char *&full, bool isLastColumn = false, bool isOptional = false) {
	auto cell = nextCell(full, isLastColumn);
	if (cell.size() == 0 && isOptional)
		return 0;

	char *end = nullptr;
	auto value = strtoul(cell.data(), &end, 10);
	if (end == nullptr || end == cell.data()) // we cannot check for '\0' as some lines have extra, unused cells...
		error("Could not extract integer from data file");
	return (uint32)value;
}

static int32 nextSint(char *&full, bool isLastColumn = false, bool isOptional = false) {
	auto cell = nextCell(full, isLastColumn);
	if (cell.size() == 0 && isOptional)
		return 0;

	char *end = nullptr;
	auto value = strtol(cell.data(), &end, 10);
	if (end == nullptr || end == cell.data()) // we cannot check for '\0' as some lines have extra, unused cells...
		error("Could not extract integer from data file");
	return (int32)value;
}

static Point nextPoint(char *&full, bool isLastColumn = false, bool isOptional = false) {
	int32 x = nextSint(full, false, isOptional);
	int32 y = nextSint(full, isLastColumn);
	return Point((int16)x, (int16)y);
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
	if (!strcmp(cell.data(), "true"))
		return true;
	if (!strcmp(cell.data(), "false"))
		return false;
	error("Could not extract bool from data file: %s", cell.data());
}

static Direction nextDirection(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	Direction result;
	if (parseDirection(cell.data(), result))
		return result;
	error("Could not extract direction from data file: %s", cell.data());
}

static GameMode nextGameMode(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	if (cell.size() == 1) {
		switch (cell[0]) {
		case '0':
			return GameMode::StartMenu;
		case '1':
			return GameMode::EdnaStd;
		case '2':
			return GameMode::Harvey;
		case '3':
			return GameMode::EdnaGirl;
		case '4':
			return GameMode::ScriptOnClick;
		case '5':
			return GameMode::DragScript;
		case '6':
			return GameMode::Zen;
		// case 7 (MainMenu) does and should not exist
		}
	}
	error("Could not extract game mode from data file: %s", cell.data());
}

static PlayerAction nextPlayerAction(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	PlayerAction result;
	if (parsePlayerAction(cell.data(), result))
		return result;
	error("Could not extract player action from data file: %s", cell.data());
}

static FontKind nextFont(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	if (strncmp("EdnaFont", cell.data(), cell.size()) == 0)
		return FontKind::EdnaFont;
	else if (strncmp("HarveyFont", cell.data(), cell.size()) == 0)
		return FontKind::HarveyFont;
	else if (strncmp("NscFontRot", cell.data(), cell.size()) == 0)
		return FontKind::NscFontRot;
	else if (strncmp("NscFontGelb", cell.data(), cell.size()) == 0)
		return FontKind::NscFontGelb;
	else if (strncmp("NscFontOrange", cell.data(), cell.size()) == 0)
		return FontKind::NscFontOrange;
	else if (strncmp("NscFontGreygreen", cell.data(), cell.size()) == 0)
		return FontKind::NscFontGreygreen;
	else if (strncmp("NscFontBlau", cell.data(), cell.size()) == 0)
		return FontKind::NscFontBlau;
	else if (strncmp("NscFontGrau", cell.data(), cell.size()) == 0)
		return FontKind::NscFontGrau;
	else if (strncmp("NscFontHellgelb", cell.data(), cell.size()) == 0)
		return FontKind::NscFontHellgelb;
	else if (strncmp("NscFontLind", cell.data(), cell.size()) == 0)
		return FontKind::NscFontLind;
	else if (strncmp("NscFontStahlblau", cell.data(), cell.size()) == 0)
		return FontKind::NscFontStahlblau;
	else if (strncmp("NscFontWeiss", cell.data(), cell.size()) == 0)
		return FontKind::NscFontWeiss;
	else if (strncmp("TestFont", cell.data(), cell.size()) == 0)
		return FontKind::TestFont;
	else if (strncmp("ActiveFont", cell.data(), cell.size()) == 0)
		return FontKind::ActiveFont;
	else if (strncmp("InactiveFont", cell.data(), cell.size()) == 0)
		return FontKind::InactiveFont;
	else if (strncmp("MenuFont", cell.data(), cell.size()) == 0)
		return FontKind::MenuFont;
	else if (strncmp("MenuFont2", cell.data(), cell.size()) == 0)
		return FontKind::MenuFont2;
	error("Could not extract font from data file: %s", cell.data());
}

DB::DB(const Path &path)
	: _path(path)
	, _scripts("script")
	, _charAnimSets("character animation set")
	, _choices("choice set", syncChoice)
	, _rooms("room")
	, _roomObjects("room object", syncRoomObject)
	, _roomObjectDisplays("room object display")
	, _roomInteractions("room interaction", syncRoomInteraction)
	, _roomItemInteractions("room item interaction", syncScriptId)
	, _roomExits("room exit")
	, _items("item", syncItem)
	, _itemInteractions("item interaction", syncScriptId)
	, _topics("topic", syncTopic)
	, _animations("animation")
	, _animationFrames("animation frame")
	, _npcs("npc")
	, _walkableAreas("walkable area")
	, _timers("timer", syncTimer)
	, _roomObjectsByRoom("room object by room", _roomObjects)
	, _ownedItemsByGameMode("owned items by gamemode", _items) {
	loadScripts();
	loadAnimations();
	loadAnimationFrames();
	loadCharAnimSets();
	loadChoices();
	loadItems();
	loadItemInteractions();
	loadRooms();
	loadRoomObjects();
	loadRoomObjectDisplays();
	loadRoomInteractions();
	loadRoomItemInteractions();
	loadRoomExits();
	loadTopics();
	loadNPCs();
	loadWalkableAreas();
	loadTimers();
}

DB::DBString::DBString(const char *string, bool ownsString)
	: _string(string), _ownsString(ownsString) {}

DB::DBString DB::DBString::ownerOf(char *string) {
	return DBString(string, true);
}

DB::DBString DB::DBString::copyOf(const char *string) {
	return DBString(scumm_strdup(string), true);
}

DB::DBString DB::DBString::refTo(const char *string) {
	return DBString(string, false);
}

DB::DBString::DBString(DBString &&other)
	: _string(other._string), _ownsString(other._ownsString) {
	other._string = nullptr;
	other._ownsString = false;
}

DB::DBString::DBString(const DBString &other)
	: _string(other._string), _ownsString(false) {}

DB::DBString &DB::DBString::operator=(DBString &&other) {
	this->~DBString();
	_string = other._string;
	_ownsString = other._ownsString;
	other._string = nullptr;
	other._ownsString = false;
	return *this;
}

DB::DBString &DB::DBString::operator=(const DBString &other) {
	this->~DBString();
	_string = other._string;
	_ownsString = false;
	return *this;
}

DB::DBString::~DBString() {
	if (_string != nullptr && _ownsString)
		free(const_cast<char *>(_string));
	_string = nullptr;
	_ownsString = false;
}

const char *DB::DBString::get() const {
	return _string == nullptr ? "" : _string;
}

bool DB::DBString::operator==(const DBString &other) const {
	return strcmp(get(), other.get()) == 0;
}

bool DB::DBString::operator!=(const DBString &other) const {
	return strcmp(get(), other.get()) != 0;
}

template<class TValue>
DB::SimpleDataSet<TValue>::SimpleDataSet(const char *typeName, RecordSyncFn<TValue> sync)
	: _typeName(typeName), _sync(sync) { }

template<class TValue>
void DB::SimpleDataSet<TValue>::set(uint32 key, const TValue &value) {
	if (_map.contains(key))
		warning("Duplicate %s: %u", _typeName, key);
	else
		_map.setVal(key, value);
}

template<class TValue>
TValue DB::SimpleDataSet<TValue>::get(uint32 key, bool required) const {
	TValue value = {};
	if (!_overlay.tryGetVal(key, value) && !_map.tryGetVal(key, value) && required)
		error("Missing %s: %u", _typeName, key);
	return value;
}

template<class TValue>
DB::TwoKeyDataSet<TValue>::TwoKeyDataSet(const char *typeName, RecordSyncFn<TValue> sync)
	: _typeName(typeName), _sync(sync) { }

template<class TValue>
void DB::TwoKeyDataSet<TValue>::set(uint32 key1, uint32 key2, const TValue &value) {
	if (_map.contains({ key1, key2 }))
		warning("Duplicate %s: %u %u", _typeName, key1, key2);
	else
		_map.setVal({ key1, key2 }, value);
}

template<class TValue>
TValue DB::TwoKeyDataSet<TValue>::get(uint32 key1, uint32 key2, bool required) const {
	TValue value = {};
	if (!_overlay.tryGetVal({ key1, key2 }, value) && !_map.tryGetVal({ key1, key2 }, value) && required)
		error("Missing %s: %u %u", _typeName, key1, key2);
	return value;
}

constexpr DB::Range::Range(uint32 begin, uint32 count)
	: _begin(begin), _count(count) {}

template<class TValue>
DB::SequenceSet<TValue>::SequenceSet(const char *typeName, RecordSyncFn<TValue> sync)
	: _typeName(typeName), _sync(sync) { }

template<class TValue>
Span<const TValue> DB::SequenceSet<TValue>::get(uint32 key, bool required) const {
	Range range;
	if (!_map.tryGetVal(key, range)) {
		if (required)
			error("Missing %s: %u", _typeName, key);
		return {};
	}
	assert(range._begin < _items.size() && range._begin + range._count <= _items.size());
	return { &_items[range._begin], range._count };
}

// necessary for db_overlay.cpp
template DB::Choice DB::SequenceSet<DB::Choice>::get(uint32 key1, uint32 key2) const;

template<class TValue>
TValue DB::SequenceSet<TValue>::get(uint32 key1, uint32 key2) const {
	const Span<const TValue> range = get(key1);
	const auto itValue = lowerBound(range.begin(), range.end(), key2,
		[](const TValue &value, uint32 key2) { return value._line < key2; });
	if (itValue == range.end() || itValue->_line != key2)
		error("Missing %s: %u %u", _typeName, key1, key2);
	return *itValue;
}

template<class TValue>
template<class GetMe, class GetParent>
void DB::SequenceSet<TValue>::setupSequences(GetMe getMe, GetParent getParent) {
	sort(_items.begin(), _items.end(), [&](const TValue &a, const TValue &b) {
		return getParent(a) != getParent(b)
			? getParent(a) < getParent(b)
			: getMe(a) < getMe(b);
	});

	uint32 begin = 0;
	for (uint32 i = 1; i < _items.size(); i++) {
		if (getParent(_items[begin]) != getParent(_items[i])) {
			_map[getParent(_items[begin])] = Range(begin, i - begin);
			begin = i;
		}
	}
	_map[getParent(_items[begin])] = Range(begin, _items.size() - begin);
}

template<class TValue>
DB::SecondaryIndex<TValue>::SecondaryIndex(const char *name, SimpleDataSet<TValue> &source)
	: DB::SequenceSet<uint32>(name)
	, _source(source) {}

template<class TValue>
template<class GetSecOrder, class GetParent>
void DB::SecondaryIndex<TValue>::build(GetSecOrder getSecOrder, GetParent getParent) {
	_map.clear();
	if (_items.size() == _source._map.size())
		_items.resize(0); // this does not free the storage
	else {
		_items.clear();
		_items.reserve(_source._map.size());
	}
	for (const auto &pair : _source._map)
		_items.push_back(pair._key);

	setupSequences(getSecOrder, getParent);
}

Span<const DB::ScriptLine> DB::script(ScriptId scriptId, bool required) const {
	return _scripts.get(scriptId, required);
}

void DB::loadScripts() {
	char *full = loadFile(_scripts._data, _path, "skript.csv");
	skipWhitespace(full);
	while (*full) {
		ScriptLine scriptLine;
		scriptLine._script = nextUint(full);
		scriptLine._line = nextUint(full);
		scriptLine._command = nextString(full);
		scriptLine._comment = nextString(full, true);
		_scripts._items.push_back(scriptLine);
	}

	_scripts.setupSequences(
		[](const ScriptLine &line) { return line._line; },
		[](const ScriptLine &line) { return line._script; });
}

DB::CharacterAnimationSet DB::characterAnimationSet(CharAnimSetId setId, ActionModeId actionModeId, bool required) const {
	return _charAnimSets.get(setId, actionModeId, required);
}

void DB::loadCharAnimSets() {
	char *full = loadFile(_charAnimSets._data, _path, "characteranimationset.csv");
	skipWhitespace(full);
	while (*full) {
		CharacterAnimationSet set;
		set._id = nextUint(full);
		set._actionMode = nextUint(full);
		set._name = nextString(full);
		set._left = nextUint(full);
		set._right = nextUint(full);
		set._down = nextUint(full);
		set._up = nextUint(full, true);
		_charAnimSets.set(set._id, set._actionMode, set);
	}
}

Span<const DB::Choice> DB::choices(ChoiceSetId choiceId) const {
	return _choices.get(choiceId);
}

DB::Choice DB::choice(ChoiceSetId setId, uint32 line) const {
	return _choices.get(setId, line);
}

void DB::loadChoices() {
	char *full = loadFile(_choices._data, _path, "choiceliste.csv");
	skipWhitespace(full);
	while (*full) {
		Choice choice;
		choice._id = nextUint(full);
		choice._line = nextUint(full);
		choice._active = nextBool(full);
		choice._text = nextString(full);
		choice._script = nextUint(full, true);
		_choices._items.push_back(choice);
	}

	_choices.setupSequences(
		[](const Choice &c) { return c._line; },
		[](const Choice &c) { return c._id; });
}

DB::Room DB::room(RoomId id, bool required) const {
	return _rooms.get(id, required);
}

void DB::loadRooms() {
	char *full = loadFile(_rooms._data, _path, "raum.csv");
	skipWhitespace(full);
	while (*full) {
		Room room;
		room._id = nextUint(full);
		room._name = nextString(full);
		room._background = nextString(full);
		room._music = nextString(full);
		if (*room._music == '/') // this messes up file lookup
			room._music++;
		room._walkAreaId = nextUint(full);
		room._vspeed = nextFloat(full);
		room._hspeed = nextFloat(full);
		room._baseYAtZeroScale = nextFloat(full);
		room._baseYAtFullScale = nextFloat(full);
		room._gameMode = nextGameMode(full);
		room._charAnimSet = nextUint(full);
		room._timer = nextUint(full, true);
		_rooms.set(room._id, room);
	}
}

DB::RoomObject DB::roomObject(RoomObjectId id, bool required) const {
	return _roomObjects.get(id, required);
}

Span<const uint32> DB::roomObjectsByRoom(RoomId id, bool required) const {
	return _roomObjectsByRoom.get(id, required);
}

void DB::loadRoomObjects() {
	char *full = loadFile(_roomObjects._data, _path, "raumobjekt.csv");
	skipWhitespace(full);
	while (*full) {
		RoomObject obj;
		obj._id = nextUint(full);
		obj._name = nextString(full);
		obj._room = nextUint(full);
		obj._pos = nextPoint(full);
		obj._posZ = nextSint(full);
		obj._image = nextString(full);
		obj._active = nextBool(full, true);
		_roomObjects.set(obj._id, obj);
	}

	_roomObjectsByRoom.build(
		[](RoomObjectId objId) { return objId; },
		[&](RoomObjectId objId) { return _roomObjects._map[objId]._room; });
}

ScriptId DB::RoomInteraction::scriptFor(PlayerAction action) const {
	switch (action) {
	case PlayerAction::None:
	case PlayerAction::Walk:
		return 0;
	case PlayerAction::Look:
		return _lookScript;
	case PlayerAction::Use:
		return _useScript;
	case PlayerAction::Talk:
		return _talkScript;
	case PlayerAction::Pick:
		return _pickScript;
	default:
		assert(false && "Player action not implemented");
		return 0;
	}
}

DB::RoomInteraction DB::roomInteraction(RoomInteractionId id, bool required) const {
	return _roomInteractions.get(id, required);
}

void DB::loadRoomInteractions() {
	char *full = loadFile(_roomInteractions._data, _path, "raumobjektinteraktion.csv");
	skipWhitespace(full);
	while (*full) {
		RoomInteraction interaction;
		interaction._id = nextUint(full);
		interaction._object = nextUint(full);
		interaction._name = nextString(full);
		interaction._walkTo = nextPoint(full);
		interaction._lookDirection = nextDirection(full);
		interaction._defaultAction = nextPlayerAction(full);
		interaction._lookScript = nextUint(full);
		interaction._useScript = nextUint(full);
		interaction._pickScript = nextUint(full);
		interaction._talkScript = nextUint(full, true);
		_roomInteractions.set(interaction._id, interaction);

		if (_roomObjects._map.contains(interaction._object))
			_roomObjects._map[interaction._object]._toInteraction = interaction._id;
	}
}

ScriptId DB::Item::scriptFor(PlayerAction action) const {
	switch (action) {
	case PlayerAction::None:
	case PlayerAction::Pick:
		return 0;
	case PlayerAction::Look:
		return _lookScript;
	case PlayerAction::Use:
		return _useScript;
	case PlayerAction::Talk:
		return _talkScript;
	default:
		assert(false && "Player action not implemented");
		return 0;
	}
}

DB::Item DB::item(ItemId id, bool required) const {
	return _items.get(id, required);
}

Span<const ItemId> DB::ownedItems(GameMode mode) const {
	return _ownedItemsByGameMode.get((uint32)mode, false);
}

void DB::loadItems() {
	char *full = loadFile(_items._data, _path, "inventarobjekt.csv");
	skipWhitespace(full);
	while (*full) {
		Item item;
		item._id = nextUint(full);
		item._gameMode = nextGameMode(full);
		item._name = nextString(full);
		item._icon = DBString::refTo(nextString(full));
		item._inventoryPos = nextUint(full);
		item._defaultAction = nextPlayerAction(full);
		item._lookScript = nextUint(full);
		item._useScript = nextUint(full);
		item._talkScript = nextUint(full, true);
		_items.set(item._id, item);
	}

	buildItemIndex();
}

void DB::buildItemIndex() {
	_ownedItemsByGameMode.build(
		[&](ItemId itemId) { return _items.get(itemId)._inventoryPos; },
		[&](ItemId itemId) {
			const Item item = _items.get(itemId);
			return item._inventoryPos == 0
				? UINT32_MAX // this splits unowned items into a dummy gamemode
				: (uint32)item._gameMode;
		}
	);
}

DB::Topic DB::topic(TopicId id, bool required) const {
	return _topics.get(id, required);
}

void DB::loadTopics() {
	char *full = loadFile(_topics._data, _path, "topic.csv");
	skipWhitespace(full);
	while (*full) {
		Topic topic;
		topic._id = nextUint(full);
		topic._roomObject = nextUint(full);
		topic._name = nextString(full);
		topic._icon = nextString(full);
		topic._inventoryPos = nextUint(full);
		topic._topicRowPos = nextUint(full);
		topic._script = nextUint(full, true);
		_topics.set(topic._id, topic);

		if (_roomObjects._map.contains(topic._id))
			_roomObjects._map[topic._id]._toTopicId = topic._id;
		if (_roomObjects._map.contains(topic._roomObject))
			_roomObjects._map[topic._roomObject]._toTopicObject = topic._roomObject;
	}
}

ScriptId DB::itemInteraction(ItemId item1, ItemId item2) const {
	ScriptId script = 0;
	_itemInteractions._map.tryGetVal({ item1, item2 }, script);
	return script;
}

ScriptId DB::roomItemInteraction(ItemId item, RoomObjectId object) const {
	ScriptId script = 0;
	_roomItemInteractions._map.tryGetVal({ item, object }, script);
	return script;
}

void DB::loadItemInteractions() {
	char *full = loadFile(_itemInteractions._data, _path, "inventarbenutzemit.csv");
	skipWhitespace(full);
	while (*full) {
		ItemId item1 = nextUint(full);
		ItemId item2 = nextUint(full);
		ScriptId script = nextUint(full);
		nextUint(full, true); // this is the actual primary key, but we don't need it
		_itemInteractions.set(item1, item2, script);
	}
}

DB::RoomExit DB::roomExit(RoomExitId id, bool required) const {
	return _roomExits.get(id, required);
}

void DB::loadRoomExits() {
	char *full = loadFile(_roomExits._data, _path, "ausgang.csv");
	skipWhitespace(full);
	while (*full) {
		RoomExit exit;
		exit._id = nextUint(full);
		exit._interaction = nextUint(full);
		exit._target = nextUint(full);
		exit._walkIn = nextPoint(full);
		exit._lookDirection = nextDirection(full, true);
		_roomExits.set(exit._id, exit);

		if (_roomInteractions._map.contains(exit._interaction)) {
			assert(_roomInteractions._map[exit._interaction]._toExit == 0);
			_roomInteractions._map[exit._interaction]._toExit = exit._id;
		}
	}
}

void DB::loadRoomItemInteractions() {
	char *full = loadFile(_roomItemInteractions._data, _path, "benutzemit.csv");
	skipWhitespace(full);
	while (*full) {
		ItemId item = nextUint(full);
		RoomObjectId object = nextUint(full);
		ScriptId script = nextUint(full, true);
		_roomItemInteractions.set(item, object, script);
	}
}

DB::Animation DB::animation(AnimationId id, bool required) const {
	return _animations.get(id, required);
}

void DB::loadAnimations() {
	char *full = loadFile(_animations._data, _path, "bildfolge.csv");
	skipWhitespace(full);
	while (*full) {
		Animation anim;
		anim._id = nextUint(full);
		anim._name = nextString(full);
		anim._duration = nextUint(full);
		anim._loop = nextBool(full, true);
		_animations.set(anim._id, anim);
	}
}

Span<const DB::AnimationFrame> DB::animationFrames(AnimationId id) const {
	return _animationFrames.get(id);
}

void DB::loadAnimationFrames() {
	char *full = loadFile(_animationFrames._data, _path, "animationsbild.csv");
	skipWhitespace(full);
	while (*full) {
		AnimationFrame frame;
		frame._frame = nextUint(full); // this is originally the sole primary key, but it is much easier to consume as sequence
		frame._animation = nextUint(full);
		frame._image = nextString(full);
		frame._altDuration = nextUint(full, true);
		_animationFrames._items.push_back(frame);
	}

	_animationFrames.setupSequences(
		[](const AnimationFrame &f) { return f._frame; },
		[](const AnimationFrame &f) { return f._animation; });
}

DB::RoomObjectDisplay DB::roomObjectDisplay(RoomObjectDisplayId id, bool required) const {
	return _roomObjectDisplays.get(id, required);
}

void DB::loadRoomObjectDisplays() {
	char *full = loadFile(_roomObjectDisplays._data, _path, "raumobjektdarstellung.csv");
	skipWhitespace(full);
	while (*full) {
		RoomObjectDisplay display;
		display._id = nextUint(full);
		display._object = nextUint(full);
		display._animation = nextUint(full, false, true);
		display._baseLineStart = nextPoint(full, false, true); // probably a mistake that this can be optional
		display._baseLineEnd = nextPoint(full, true);
		_roomObjectDisplays.set(display._id, display);

		if (_roomObjects._map.contains(display._object))
			_roomObjects._map[display._object]._toDisplay = display._id;
	}
}

DB::NPC DB::npc(NPCId id, bool required) const {
	return _npcs.get(id, required);
}

void DB::loadNPCs() {
	char *full = loadFile(_npcs._data, _path, "nsc.csv");
	skipWhitespace(full);
	while (*full) {
		NPC npc;
		npc._id = nextUint(full);
		npc._object = nextUint(full);
		npc._charAnimSet = nextUint(full);
		npc._name = nextString(full);
		npc._font = nextFont(full);
		npc._vspeed = nextFloat(full);
		npc._hspeed = nextFloat(full);
		npc._baseYAtZeroScale = nextFloat(full);
		npc._baseYAtFullScale = nextFloat(full, true);
		_npcs.set(npc._id, npc);

		if (_roomObjects._map.contains(npc._object))
			_roomObjects._map[npc._object]._toNPC = npc._id;
	}
}

DB::WalkableArea DB::walkableArea(WalkableAreaId id) const {
	return _walkableAreas.get(id);
}

void DB::loadWalkableAreas() {
	char *full = loadFile(_walkableAreas._data, _path, "walkableareamap.csv");
	skipWhitespace(full);
	while (*full) {
		WalkableArea area;
		area._id = nextUint(full);
		area._room = nextUint(full);
		area._file = nextString(full, true);
		_walkableAreas.set(area._id, area);
	}
}

DB::Timer DB::timer(TimerId id) const {
	return _timers.get(id);
}

void DB::loadTimers() {
	char *full = loadFile(_timers._data, _path, "timer.csv");
	skipWhitespace(full);
	while (*full) {
		Timer timer;
		timer._id = nextUint(full);
		timer._script = nextUint(full);
		timer._duration = nextUint(full);
		timer._active = nextBool(full, true);
		_timers.set(timer._id, timer);
	}
}

}

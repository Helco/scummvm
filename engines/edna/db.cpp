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
	if (cell.size() == 1) {
		switch (tolower(cell[0])) {
		case 'n':
			return Direction::Up;
		case 's':
			return Direction::Down;
		case 'o': // this is a default case and the developers apparently could not decide
		case 'e': // whether to use english or german compass directions or enum numbers...
		case '3': 
			return Direction::Right;
		case 'w':
			return Direction::Left;
		}
	}
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
	if (cell.size() == 1) {
		switch (cell[0]) {
		case '0':
			return PlayerAction::None;
		case 'a':
		case 'l':
			return PlayerAction::Look;
		case 'b':
		case 'u':
			return PlayerAction::Use;
		case 'n':
		case 'p':
			return PlayerAction::Take;
		case 'r':
		case 't':
			return PlayerAction::Talk;
		}
	}
	error("Could not extract player action from data file: %s", cell.data());
}

static Font nextFont(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	if (strncmp("EdnaFont", cell.data(), cell.size()) == 0)
		return Font::EdnaFont;
	else if (strncmp("HarveyFont", cell.data(), cell.size()) == 0)
		return Font::HarveyFont;
	else if (strncmp("NscFontRot", cell.data(), cell.size()) == 0)
		return Font::NscFontRot;
	else if (strncmp("NscFontGelb", cell.data(), cell.size()) == 0)
		return Font::NscFontGelb;
	else if (strncmp("NscFontOrange", cell.data(), cell.size()) == 0)
		return Font::NscFontOrange;
	else if (strncmp("NscFontGreygreen", cell.data(), cell.size()) == 0)
		return Font::NscFontGreygreen;
	else if (strncmp("NscFontBlau", cell.data(), cell.size()) == 0)
		return Font::NscFontBlau;
	else if (strncmp("NscFontGrau", cell.data(), cell.size()) == 0)
		return Font::NscFontGrau;
	else if (strncmp("NscFontHellgelb", cell.data(), cell.size()) == 0)
		return Font::NscFontHellgelb;
	else if (strncmp("NscFontLind", cell.data(), cell.size()) == 0)
		return Font::NscFontLind;
	else if (strncmp("NscFontStahlblau", cell.data(), cell.size()) == 0)
		return Font::NscFontStahlblau;
	else if (strncmp("NscFontWeiss", cell.data(), cell.size()) == 0)
		return Font::NscFontWeiss;
	else if (strncmp("TestFont", cell.data(), cell.size()) == 0)
		return Font::TestFont;
	else if (strncmp("ActiveFont", cell.data(), cell.size()) == 0)
		return Font::ActiveFont;
	else if (strncmp("InactiveFont", cell.data(), cell.size()) == 0)
		return Font::InactiveFont;
	else if (strncmp("MenuFont", cell.data(), cell.size()) == 0)
		return Font::MenuFont;
	else if (strncmp("MenuFont2", cell.data(), cell.size()) == 0)
		return Font::MenuFont2;
	error("Could not extract font from data file: %s", cell.data());
}

DB::DB(const Path &path)
	: path(path)
	, _scripts("script")
	, _charAnimSets("character animation set")
	, _choices("choice set")
	, _rooms("room")
	, _roomObjects("room object")
	, _roomObjectDisplays("room object display")
	, _roomInteractions("room interaction")
	, _roomItemInteractions("room item interaction")
	, _roomExits("room exit")
	, _items("item")
	, _itemInteractions("item interaction")
	, _topics("topic")
	, _animations("animation")
	, _animationFrames("animation frame")
	, _npcs("npc")
	, _walkableAreas("walkable area")
	, _timers("timer")
	, _roomObjectsByRoom("room object by room", _roomObjects) {
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

DB::~DB() {}

template<class TValue>
DB::SimpleDataSet<TValue>::SimpleDataSet(const char *typeName) : _typeName(typeName) { }

template<class TValue>
void DB::SimpleDataSet<TValue>::set(uint32 key, const TValue &value) {
	if (_map.contains(key))
		warning("Duplicate %s: %u", _typeName, key);
	else
		_map.setVal(key, value);
}

template<class TValue>
TValue DB::SimpleDataSet<TValue>::get(uint32 key, bool required) const {
	TValue value;
	if (!_map.tryGetVal(key, value)) {
		if (required)
			error("Missing %s: %u", _typeName, key);
	}
	return value;
}

template<class TValue>
DB::TwoKeyDataSet<TValue>::TwoKeyDataSet(const char *typeName) : _typeName(typeName) { }

template<class TValue>
void DB::TwoKeyDataSet<TValue>::set(uint32 key1, uint32 key2, const TValue &value) {
	if (_map.contains({ key1, key2 }))
		warning("Duplicate %s: %u %u", _typeName, key1, key2);
	else
		_map.setVal({ key1, key2 }, value);
}

template<class TValue>
TValue DB::TwoKeyDataSet<TValue>::get(uint32 key1, uint32 key2) const {
	TValue value;
	if (!_map.tryGetVal({ key1, key2 }, value ))
		error("Missing %s: %u %u", _typeName, key1, key2);
	return value;
}

template<class TValue>
DB::SequenceSet<TValue>::SequenceSet(const char *typeName) : _typeName(typeName) { }

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
			_map[getParent(_items[begin])] = {begin, i - begin};
			begin = i;
		}
	}
	_map[getParent(_items[begin])] = { begin, _items.size() - begin };
}

template<class TValue>
DB::SecondaryIndex<TValue>::SecondaryIndex(const char *name, SimpleDataSet<TValue> &source)
	: DB::SequenceSet<uint32>(name)
	, _source(source) {}

template<class TValue>
void DB::SecondaryIndex<TValue>::build(PointerToID toParent) {
	_map.clear();
	_items.clear();
	_items.reserve(_source._map.size());
	for (const auto &pair : _source._map)
		_items.push_back(pair._key);

	setupSequences(
		[](const uint32 &id) { return id; },
		[&](const uint32 &id) { return _source._map[id].*toParent; });
}

Span<const DB::ScriptLine> DB::script(ScriptId scriptId) const {
	return _scripts.get(scriptId);
}

void DB::loadScripts() {
	char *full = loadFile(_scripts._data, path, "skript.csv");
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

DB::CharacterAnimationSet DB::characterAnimationSet(CharAnimSetId setId, ActionModeId actionModeId) const {
	return _charAnimSets.get(setId, actionModeId);
}

void DB::loadCharAnimSets() {
	char *full = loadFile(_charAnimSets._data, path, "characteranimationset.csv");
	skipWhitespace(full);
	while (*full) {
		CharacterAnimationSet set;
		set._id = nextUint(full);
		set._actionMode = nextUint(full);
		set._name = nextString(full);
		set._left = nextUint(full);
		set._right = nextUint(full);
		set._forward = nextUint(full);
		set._back = nextUint(full, true);
		_charAnimSets.set(set._id, set._actionMode, set);
	}
}

Span<const DB::Choice> DB::choices(ChoiceSetId choiceId) const {
	return _choices.get(choiceId);
}

void DB::loadChoices() {
	char *full = loadFile(_choices._data, path, "choiceliste.csv");
	skipWhitespace(full);
	while (*full) {
		Choice choice;
		choice._set = nextUint(full);
		choice._line = nextUint(full);
		choice._active = nextBool(full);
		choice._text = nextString(full);
		choice._script = nextUint(full, true);
		_choices._items.push_back(choice);
	}

	_choices.setupSequences(
		[](const Choice &c) { return c._line; },
		[](const Choice &c) { return c._set; });
}

DB::Room DB::room(RoomId id, bool required) const {
	return _rooms.get(id, required);
}

void DB::loadRooms() {
	char *full = loadFile(_rooms._data, path, "raum.csv");
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
	char *full = loadFile(_roomObjects._data, path, "raumobjekt.csv");
	skipWhitespace(full);
	while (*full) {
		RoomObject obj;
		obj._id = nextUint(full);
		obj._name = nextString(full);
		obj._room = nextUint(full);
		obj._posX = nextSint(full);
		obj._posY = nextSint(full);
		obj._posZ = nextSint(full);
		obj._image = nextString(full);
		obj._active = nextBool(full, true);
		_roomObjects.set(obj._id, obj);
	}

	_roomObjectsByRoom.build(&RoomObject::_room);
}

DB::RoomInteraction DB::roomInteraction(RoomInteractionId id, bool required) const {
	return _roomInteractions.get(id, required);
}

void DB::loadRoomInteractions() {
	char *full = loadFile(_roomInteractions._data, path, "raumobjektinteraktion.csv");
	skipWhitespace(full);
	while (*full) {
		RoomInteraction interaction;
		interaction._id = nextUint(full);
		interaction._object = nextUint(full);
		interaction._name = nextString(full);
		interaction._walkToX = nextSint(full);
		interaction._walkToY = nextSint(full);
		interaction._lookDirection = nextDirection(full);
		interaction._defaultAction = nextPlayerAction(full);
		interaction._lookScript = nextUint(full);
		interaction._useScript = nextUint(full);
		interaction._takeScript = nextUint(full);
		interaction._talkScript = nextUint(full, true);
		_roomInteractions.set(interaction._id, interaction);

		if (_roomObjects._map.contains(interaction._object))
			_roomObjects._map[interaction._object]._toInteraction = interaction._id;
	}
}

DB::Item DB::item(ItemId id) const {
	return _items.get(id);
}

void DB::loadItems() {
	char *full = loadFile(_items._data, path, "inventarobjekt.csv");
	skipWhitespace(full);
	while (*full) {
		Item item;
		item._id = nextUint(full);
		item._gameMode = nextGameMode(full);
		item._name = nextString(full);
		item._icon = nextString(full);
		item._inventoryPos = nextUint(full);
		item._defaultAction = nextPlayerAction(full);
		item._lookScript = nextUint(full);
		item._useScript = nextUint(full);
		item._talkScript = nextUint(full, true);
		_items.set(item._id, item);
	}
}

DB::Topic DB::topic(TopicId id) const {
	return _topics.get(id);
}

void DB::loadTopics() {
	char *full = loadFile(_topics._data, path, "topic.csv");
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
	char *full = loadFile(_itemInteractions._data, path, "inventarbenutzemit.csv");
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
	char *full = loadFile(_roomExits._data, path, "ausgang.csv");
	skipWhitespace(full);
	while (*full) {
		RoomExit exit;
		exit._id = nextUint(full);
		exit._interaction = nextUint(full);
		exit._target = nextUint(full);
		exit._walkToX = nextSint(full);
		exit._walkToY = nextSint(full);
		exit._lookDirection = nextDirection(full, true);
		_roomExits.set(exit._id, exit);

		if (_roomInteractions._map.contains(exit._interaction))
			_roomInteractions._map[exit._interaction]._toExit = exit._id;
	}
}

void DB::loadRoomItemInteractions() {
	char *full = loadFile(_roomItemInteractions._data, path, "benutzemit.csv");
	skipWhitespace(full);
	while (*full) {
		ItemId item = nextUint(full);
		RoomObjectId object = nextUint(full);
		ScriptId script = nextUint(full, true);
		_roomItemInteractions.set(item, object, script);
	}
}

DB::Animation DB::animation(AnimationId id) const {
	return _animations.get(id);
}

void DB::loadAnimations() {
	char *full = loadFile(_animations._data, path, "bildfolge.csv");
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
	char *full = loadFile(_animationFrames._data, path, "animationsbild.csv");
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
	char *full = loadFile(_roomObjectDisplays._data, path, "raumobjektdarstellung.csv");
	skipWhitespace(full);
	while (*full) {
		RoomObjectDisplay display;
		display._id = nextUint(full);
		display._object = nextUint(full);
		display._animation = nextUint(full, false, true);
		display._startX = nextSint(full, false, true); // probably a mistake that this can be optional
		display._startY = nextSint(full);
		display._endX = nextSint(full);
		display._endY = nextSint(full, true);
		_roomObjectDisplays.set(display._id, display);

		if (_roomObjects._map.contains(display._object))
			_roomObjects._map[display._object]._toDisplay = display._id;
	}
}

DB::NPC DB::npc(NPCId id, bool required) const {
	return _npcs.get(id, required);
}

void DB::loadNPCs() {
	char *full = loadFile(_npcs._data, path, "nsc.csv");
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

		if (_roomObjects._map.contains(npc._id))
			_roomObjects._map[npc._id]._toNPC = npc._id;
	}
}

DB::WalkableArea DB::walkableArea(WalkableAreaId id) const {
	return _walkableAreas.get(id);
}

void DB::loadWalkableAreas() {
	char *full = loadFile(_walkableAreas._data, path, "walkableareamap.csv");
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
	char *full = loadFile(_timers._data, path, "timer.csv");
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

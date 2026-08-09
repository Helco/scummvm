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

#ifndef EDNA_DB_H
#define EDNA_DB_H

#include "edna/scriptcommand.h"

#include "common/serializer.h"

namespace Edna {

using FileData = Common::SpanOwner<StringSpan>;
class Console;

class DB final {
	struct StringBuffer;
public:
	DB(const Common::Path &path);

	void resetOverlay(); ///< implemented in db_overlay.cpp
	void syncOverlay(Common::Serializer &s); 
	uint32 validate(); ///< implemented in db_validation.cpp

	// custom string that supports foreign references
	// i.e. either mostly ref into FileData or rarely own strings through overlay
	// only necessary for strings that could change
	class DBString {
		const char *_string = nullptr;
		bool _ownsString = false;
		DBString(const char *string, bool ownsString);
		static DBString ownerOf(char *string);
		static DBString copyOf(const char *string);
		static DBString refTo(const char *string);
		friend class DB;
	public:
		DBString() = default;
		DBString(DBString &&other);
		DBString(const DBString &other);
		DBString &operator=(DBString &&other);
		DBString &operator=(const DBString &other);
		~DBString();

		const char *get() const;
		bool operator==(const DBString &other) const;
		bool operator!=(const DBString &other) const;
	};

	struct ScriptLine {
		ScriptId _script = 0;
		uint32 _line = 0;
		ScriptCommand _command = {};
		const char *_comment = {};
	};
	Common::Span<const ScriptLine> script(ScriptId scriptId, bool required = true) const;

	struct CharacterAnimationSet {
		CharAnimSetId _id = 0;
		ActionModeId _actionMode = 0;
		const char *_name = "";
		AnimationId _left = 0;
		AnimationId _right = 0;
		AnimationId _down = 0;
		AnimationId _up = 0;
	};
	CharacterAnimationSet characterAnimationSet(CharAnimSetId set, ActionModeId actionMode, bool required = true) const;

	struct Choice {
		ChoiceSetId _id = 0;
		uint32 _line = 0;
		bool _active = false;
		const char *_text = "";
		ScriptId _script = 0;
	};
	Common::Span<const Choice> choices(ChoiceSetId choiceId) const;
	Choice choice(ChoiceSetId setId, uint32 line) const;
	void setChoiceScript(ChoiceSetId setId, uint32 line, ScriptId scriptId);
	void toggleChoice(ChoiceSetId setId, uint32 line, bool active);

	struct Room {
		RoomId _id = 0;
		const char *_name = "";
		const char *_background = "";
		const char *_music = "";
		WalkableAreaId _walkAreaId = 0;
		float _vspeed = 0;
		float _hspeed = 0;
		float _baseYAtZeroScale = 0;
		float _baseYAtFullScale = 0;
		GameMode _gameMode = {};
		CharAnimSetId _charAnimSet = 0;
		TimerId _timer = 0;
	};
	Room room(RoomId id, bool required = true) const;

	struct RoomObject {
		RoomObjectId _id = 0;
		const char *_name = "";
		RoomId _room = 0;
		Common::Point _pos;
		int32 _posZ = 0; ///< unused
		const char *_image = "";
		bool _active = false;

		// associations
		RoomInteractionId _toInteraction = 0;
		RoomObjectDisplayId _toDisplay = 0;
		TopicId _toTopicId = 0;
		TopicId _toTopicObject = 0;
		NPCId _toNPC = 0;
	};
	RoomObject roomObject(RoomObjectId id, bool required = true) const;
	Common::Span<const RoomObjectId> roomObjectsByRoom(RoomId id, bool required = true) const;
	void setRoomObjectPos(RoomObjectId id, Common::Point pos);
	void toggleRoomObject(RoomObjectId id, bool active);

	struct RoomObjectDisplay {
		RoomObjectDisplayId _id = 0;
		RoomObjectId _object = 0;
		AnimationId _animation = 0;
		Common::Point _baseLineStart, _baseLineEnd;
	};
	RoomObjectDisplay roomObjectDisplay(RoomObjectDisplayId id, bool required = true) const;

	struct RoomInteraction {
		RoomInteractionId _id = 0;
		RoomObjectId _object = 0;
		const char *_name = "";
		Common::Point _walkTo;
		Direction _lookDirection = {};
		PlayerAction _defaultAction = {};
		ScriptId _lookScript = 0;
		ScriptId _useScript = 0;
		ScriptId _pickScript = 0;
		ScriptId _talkScript = 0;

		// associations
		RoomExitId _toExit = 0;

		ScriptId scriptFor(PlayerAction action) const;
	};
	RoomInteraction roomInteraction(RoomInteractionId id, bool required = true) const;
	void setRoomInteractionScript(RoomInteractionId id, PlayerAction action, ScriptId scriptId);

	struct Item {
		ItemId _id = 0;
		GameMode _gameMode = {};
		const char *_name = "";
		DBString _icon = {};
		uint32 _inventoryPos = 0; ///< TODO: could this be used for "is in inventory?
		PlayerAction _defaultAction = {};
		ScriptId _lookScript = 0;
		ScriptId _useScript = 0;
		ScriptId _talkScript = 0;

		ScriptId scriptFor(PlayerAction action) const;
	};
	Item item(ItemId id, bool required = true) const;
	Common::Span<const ItemId> ownedItems(GameMode mode) const;
	void buildItemIndex(); ///< has to be called after setItemPos otherwise ownedItems will be wrong
	void setItemPos(ItemId id, uint32 pos);
	void setItemScript(ItemId itemId, PlayerAction action, ScriptId scriptId);
	void setItemIcon(ItemId itemId, const char *newIcon);

	// a topic is a kind of item only used for when controlling Harvey
	struct Topic {
		TopicId _id = 0; ///< this is somehow also a room object id?
		RoomObjectId _roomObject = 0;
		const char *_name = "";
		const char *_icon = "";
		uint32 _inventoryPos = 0;
		uint32 _topicRowPos = 0;
		ScriptId _script = 0;
	};
	Topic topic(TopicId id, bool required = true) const;

	ScriptId itemInteraction(ItemId item1, ItemId item2) const; ///< can return 0
	ScriptId roomItemInteraction(ItemId item, RoomObjectId object) const; ///< can return 0
	void setItemInteraction(ItemId item1, ItemId item2, ScriptId scriptId);
	void setRoomItemInteraction(ItemId item, RoomObjectId object, ScriptId scriptId);

	struct RoomExit {
		RoomExitId _id = 0;
		RoomInteractionId _interaction = 0;
		RoomId _target = 0;
		Common::Point _walkIn;
		Direction _lookDirection = {};
	};
	RoomExit roomExit(RoomExitId id, bool required = true) const;

	struct Animation {
		AnimationId _id = 0;
		const char *_name = "";
		uint32 _duration = 0;
		bool _loop = false;
	};
	Animation animation(AnimationId id, bool required = true) const;

	struct AnimationFrame {
		AnimationFrameId _frame = 0;
		AnimationId _animation = 0;
		const char *_image = "";
		uint32 _altDuration = 0;
	};
	Common::Span<const AnimationFrame> animationFrames(AnimationId id) const;

	struct NPC {
		NPCId _id = 0;
		RoomObjectId _object = 0;
		CharAnimSetId _charAnimSet = 0;
		const char *_name = "";
		FontKind _font = {};
		float _vspeed = 0;
		float _hspeed = 0;
		float _baseYAtZeroScale = 0;
		float _baseYAtFullScale = 0;
	};
	NPC npc(NPCId id, bool required = true) const;

	struct WalkableArea {
		WalkableAreaId _id = 0;
		RoomId _room = 0; // no foreign key necessary, room has an explicit attribute
		const char *_file = "";
	};
	WalkableArea walkableArea(WalkableAreaId id) const;

	struct Timer {
		TimerId _id = 0;
		ScriptId _script = 0;
		uint32 _duration = 0;
		bool _active = false;
	};
	Timer timer(TimerId id) const;
	void toggleTimer(TimerId id, bool active);

private:
	friend class Console; ///< for debugging output

	// When loading value will be the original value, 
	// only modifiable values should be loaded or saved
	template<class TValue>
	using RecordSyncFn = void (*)(TValue &value, Common::Serializer &s);

	// For singular data referenced by one integer key
	template<class TValue>
	struct SimpleDataSet {
		const char *const _typeName;
		const RecordSyncFn<TValue> _sync;
		FileData _data;
		Common::HashMap<uint32, TValue> _map;
		Common::HashMap<uint32, TValue> _overlay;

		SimpleDataSet(const char *typeName, RecordSyncFn<TValue> sync = nullptr);
		void set(uint32 key, const TValue &value);
		TValue get(uint32 key, bool required = true) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const;
		void overlay(const TValue &value);
		void ensureOverlay(uint32 key);
		void resetOverlay();
		void sync(Common::Serializer &s);
	};

	// For singular data referenced by two integer keys
	template<class TValue>
	struct TwoKeyDataSet {
		const char *const _typeName;
		const RecordSyncFn<TValue> _sync;
		FileData _data;
		TwoKeyMap<TValue> _map;
		TwoKeyMap<TValue> _overlay;

		TwoKeyDataSet(const char *typeName, RecordSyncFn<TValue> sync = nullptr);
		void set(uint32 key1, uint32 key2, const TValue &value);
		TValue get(uint32 key1, uint32 key2, bool required = true) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const; ///< Checks that some pair starting with key exists
		void overlay(uint32 key1, uint32 key2, const TValue &value);
		void resetOverlay();
		void sync(Common::Serializer &s);
	};

	// For a sequence referenced by one key
	struct Range {
		Range() = default;
		constexpr Range(uint32 begin, uint32 count);
		uint32 _begin = 0;
		uint32 _count = 0;
	};
	template<class TValue>
	struct SequenceSet {
		const char *const _typeName;
		const RecordSyncFn<TValue> _sync;
		FileData _data;
		Common::Array<TValue> _items;
		Common::HashMap<uint32, Range> _map;
		Common::HashMap<uint32, TValue> _backup; ///< for SequenceSet we write the *original* values into the separate map
		// this is so we can still return a span of (modified) items

		SequenceSet(const char *typeName, RecordSyncFn<TValue> sync = nullptr);
		template<class GetMe, class GetParent>
		void setupSequences(GetMe getMe, GetParent getParent);
		Common::Span<const TValue> get(uint32 key, bool required = true) const;
		TValue get(uint32 key1, uint32 key2) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const;
		uint32 validateRef(uint32 key1, uint32 key2, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const;
		uint32 getItemIndex(uint32 set, uint32 line) const; ///< returns UINT32_MAX if the entry does not exist
		void overlay(const TValue &value);
		void resetOverlay();
		void sync(Common::Serializer &s);
	};

	// For faster queries and easier data structures
	template<class TValue>
	struct SecondaryIndex : public SequenceSet<uint32> {
		SimpleDataSet<TValue> &_source;

		SecondaryIndex(const char *name, SimpleDataSet<TValue> &source);
		template<class GetSecOrder, class GetParent>
		void build(GetSecOrder getSecOrder, GetParent getParent);
	};

	void loadScripts();
	void loadCharAnimSets();
	void loadChoices();
	void loadRooms();
	void loadRoomObjects();
	void loadRoomObjectDisplays();
	void loadRoomInteractions();
	void loadRoomItemInteractions();
	void loadRoomExits();
	void loadItems();
	void loadItemInteractions();
	void loadTopics();
	void loadAnimations();
	void loadAnimationFrames();
	void loadNPCs();
	void loadWalkableAreas();
	void loadTimers();

	static void syncDBString(DBString &value, Common::Serializer &s);
	static void syncChoice(Choice &value, Common::Serializer &s);
	static void syncItem(Item &value, Common::Serializer &s);
	static void syncScriptId(ScriptId &value, Common::Serializer &s);
	static void syncRoomObject(RoomObject &value, Common::Serializer &s);
	static void syncRoomInteraction(RoomInteraction &value, Common::Serializer &s);
	static void syncTopic(Topic &value, Common::Serializer &s);
	static void syncTimer(Timer &value, Common::Serializer &s);

	uint32 validateScripts() const;
	uint32 validateCharAnimSets() const;
	uint32 validateChoices() const;
	uint32 validateRooms() const;
	uint32 validateRoomObjects() const;
	uint32 validateRoomObjectDisplays() const;
	uint32 validateRoomInteractions() const;
	uint32 validateRoomItemInteractions() const;
	uint32 validateRoomExits() const;
	uint32 validateItems() const;
	uint32 validateItemInteractions() const;
	uint32 validateTopics() const;
	uint32 validateAnimations() const;
	uint32 validateAnimationFrames() const;
	uint32 validateNPCs() const;
	uint32 validateWalkableAreas() const;
	uint32 validateTimers() const;
	uint32 validateNpcByRoomObject(RoomObjectId objectId, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const;
	uint32 validateScriptCommand(const ScriptLine &line) const;

	static uint32 validateOptPath(
		const char *path,
		const char *sourceType, uint32 sourceKey,
		const char *basePath = "", const char *ext = "");
	static uint32 validatePath(
		const char *path,
		const char *sourceType, uint32 sourceKey,
		const char *basePath = "", const char *ext = "");

	const Common::Path _path;
	SequenceSet<ScriptLine> _scripts;
	TwoKeyDataSet<CharacterAnimationSet> _charAnimSets;
	SequenceSet<Choice> _choices;
	SimpleDataSet<Room> _rooms;
	SimpleDataSet<RoomObject> _roomObjects;
	SimpleDataSet<RoomObjectDisplay> _roomObjectDisplays;
	SimpleDataSet<RoomInteraction> _roomInteractions;
	TwoKeyDataSet<ScriptId> _roomItemInteractions;
	SimpleDataSet<RoomExit> _roomExits;
	SimpleDataSet<Item> _items;
	TwoKeyDataSet<ScriptId> _itemInteractions;
	SimpleDataSet<Topic> _topics;
	SimpleDataSet<Animation> _animations;
	SequenceSet<AnimationFrame> _animationFrames;
	SimpleDataSet<NPC> _npcs;
	SimpleDataSet<WalkableArea> _walkableAreas;
	SimpleDataSet<Timer> _timers;

	SecondaryIndex<RoomObject> _roomObjectsByRoom;
	SecondaryIndex<Item> _ownedItemsByGameMode;
};

}

#endif // EDNA_DB_H

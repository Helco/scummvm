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

namespace Edna {

using FileData = Common::SpanOwner<StringSpan>;

class DB {
public:
	DB(const Common::Path &path);
	~DB();

	uint32 validate(); ///< implemented in db_validation.cpp

	struct ScriptLine {
		ScriptId _script = 0;
		uint32 _line = 0;
		ScriptCommand _command;
		const char *_comment = "";
	};
	Common::Span<const ScriptLine> script(ScriptId scriptId) const;

	struct CharacterAnimationSet {
		CharAnimSetId _id = 0;
		ActionModeId _actionMode = 0;
		const char *_name = "";
		AnimationId _left = 0;
		AnimationId _right = 0;
		AnimationId _forward = 0;
		AnimationId _back = 0;
	};
	CharacterAnimationSet characterAnimationSet(CharAnimSetId set, ActionModeId actionMode, bool required = true) const;

	struct Choice {
		ChoiceSetId _set = 0;
		uint32 _line = 0;
		bool _active = false;
		const char *_text = "";
		ScriptId _script = 0;
	};
	Common::Span<const Choice> choices(ChoiceSetId choiceId) const;

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
		int32 _posX = 0;
		int32 _posY = 0;
		int32 _posZ = 0;
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
	Common::Span<const uint32> roomObjectsByRoom(RoomId id, bool required = true) const;

	struct RoomObjectDisplay {
		RoomObjectDisplayId _id = 0;
		RoomObjectId _object = 0;
		AnimationId _animation = 0;
		int32 _startX = 0;
		int32 _startY = 0;
		int32 _endX = 0;
		int32 _endY = 0;
	};
	RoomObjectDisplay roomObjectDisplay(RoomObjectDisplayId id, bool required = true) const;

	struct RoomInteraction {
		RoomInteractionId _id = 0;
		RoomObjectId _object = 0;
		const char *_name = "";
		int32 _walkToX = 0;
		int32 _walkToY = 0;
		Direction _lookDirection = {};
		PlayerAction _defaultAction = {};
		ScriptId _lookScript = 0;
		ScriptId _useScript = 0;
		ScriptId _takeScript = 0;
		ScriptId _talkScript = 0;

		// associations
		RoomExitId _toExit = 0;
	};
	RoomInteraction roomInteraction(RoomInteractionId id, bool required = true) const;

	struct Item {
		ItemId _id = 0;
		GameMode _gameMode = {};
		const char *_name = "";
		const char *_icon = "";
		uint32 _inventoryPos = 0;
		PlayerAction _defaultAction = {};
		ScriptId _lookScript = 0;
		ScriptId _useScript = 0;
		ScriptId _talkScript = 0;
	};
	Item item(ItemId id) const;

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
	Topic topic(TopicId id) const;

	ScriptId itemInteraction(ItemId item1, ItemId item2) const; ///< can return 0
	ScriptId roomItemInteraction(ItemId item, RoomObjectId object) const; ///< can return 0

	struct RoomExit {
		RoomExitId _id = 0;
		RoomInteractionId _interaction = 0;
		RoomId _target = 0;
		int32 _walkToX = 0;
		int32 _walkToY = 0;
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
		Font _font = {};
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

private:
	// For singular data referenced by one integer key
	template<class TValue>
	struct SimpleDataSet {
		const char *const _typeName;
		FileData _data;
		Common::HashMap<uint32, TValue> _map;

		SimpleDataSet(const char *typeName);
		void set(uint32 key, const TValue &value);
		TValue get(uint32 key, bool required = true) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const;
	};

	// For singular data referenced by two integer keys
	using TwoKey = Common::Pair<uint32, uint32>;
	struct TwoKeyHash {
		uint operator()(const TwoKey &key) const {
			return (uint)(key.first ^ key.second);
		}
	};
	struct TwoKeyEqualTo {
		bool operator()(const TwoKey &a, const TwoKey &b) const {
			return a.first == b.first && a.second == b.second;
		}
	};
	template<class TValue>
	using TwoKeyMap = Common::HashMap<TwoKey, TValue, TwoKeyHash, TwoKeyEqualTo>;
	template<class TValue>
	struct TwoKeyDataSet {
		const char *const _typeName;
		FileData _data;
		TwoKeyMap<TValue> _map;

		TwoKeyDataSet(const char *typeName);
		void set(uint32 key1, uint32 key2, const TValue &value);
		TValue get(uint32 key1, uint32 key2, bool required = true) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const; ///< Checks that some pair starting with key exists
	};

	// For a sequence referenced by one key
	struct Range {
		uint32 _begin = 0;
		uint32 _count = 0;
	};
	template<class TValue>
	struct SequenceSet {
		const char *const _typeName;
		FileData _data;
		Common::Array<TValue> _items;
		Common::HashMap<uint32, Range> _map;

		SequenceSet(const char *typeName);
		template<class GetMe, class GetParent>
		void setupSequences(GetMe, GetParent getParent);
		Common::Span<const TValue> get(uint32 key, bool required = true) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey) const;
		uint32 validateRef(uint32 key, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const;
		uint32 validateRef(uint32 key1, uint32 key2, const char *sourceType, uint32 sourceKey1, uint32 sourceKey2) const;
	};

	// For faster queries and easier data structures
	template<class TValue>
	struct SecondaryIndex : public SequenceSet<uint32> {
		using PointerToID = uint32 TValue::*;
		SimpleDataSet<TValue> &_source;

		SecondaryIndex(const char *name, SimpleDataSet<TValue> &source);
		void build(PointerToID toParent);
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
	uint32 validateScriptCommand(const ScriptLine &line) const;

	static uint32 validateOptPath(
		const char *path,
		const char *sourceType, uint32 sourceKey,
		const char *basePath = "", const char *ext = "");
	static uint32 validatePath(
		const char *path,
		const char *sourceType, uint32 sourceKey,
		const char *basePath = "", const char *ext = "");

	const Common::Path path;
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
};

}

#endif // EDNA_DB_H

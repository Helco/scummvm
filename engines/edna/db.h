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

#include "common/span.h"

namespace Edna {

using FileData = Common::SpanOwner<Common::Span<char>>;

using ScriptId = uint32;
using CharAnimSetId = uint32;
using ActionModeId = uint32;
using ImageSequenceId = uint32;
using ChoiceSetId = uint32;
using RoomId = uint32;
using WalkableAreaId = uint32;
using TimerId = uint32;
using GuiId = uint32; // not a DB data

class DB final {
public:
	DB(const Common::Path &path);
	~DB();

	struct ScriptLine {
		ScriptId _id = 0;
		uint32 _line = 0;
		const char *_command = "";
		const char *_comment = "";
	};
	Common::Span<const ScriptLine> script(ScriptId scriptId) const;

	struct CharacterAnimationSet {
		CharAnimSetId _id = 0;
		ActionModeId _actionMode = 0;
		const char *_name = "";
		ImageSequenceId _left = 0;
		ImageSequenceId _right = 0;
		ImageSequenceId _forward = 0;
		ImageSequenceId _back = 0;
	};
	CharacterAnimationSet characterAnimationSet(CharAnimSetId set, ActionModeId actionMode) const;

	struct Choice {
		ChoiceSetId _id;
		uint32 _line;
		bool _active;
		const char *_text;
		ScriptId _script;
	};
	Common::Span<const Choice> choices(ChoiceSetId choiceId) const;

	struct Room {
		RoomId _id;
		const char *_name;
		const char *_background;
		const char *_music;
		WalkableAreaId _walkAreaId;
		float _vspeed;
		float _hspeed;
		float _baseYAtZeroScale;
		float _baseYAtFullScale;
		GuiId _guiId;
		CharAnimSetId _charAnimSet;
		TimerId _timer;
	};
	Room room(RoomId id) const;

private:
	// For singular data referenced by one integer key
	template<class TValue>
	struct SimpleDataSet {
		FileData _data;
		Common::HashMap<uint32, TValue> _map;

		TValue get(uint32 key, const char *name) const;
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
	struct TwoKeyDataSet {
		FileData _data;
		Common::HashMap<TwoKey, TValue, TwoKeyHash, TwoKeyEqualTo> _map;

		TValue get(uint32 key1, uint32 key2, const char *name) const;
	};

	// For a sequence referenced by one key
	struct Range {
		uint32 _begin = 0;
		uint32 _count = 0;
	};
	template<class TValue>
	struct SequenceSet {
		FileData _data;
		Common::Array<TValue> _items;
		Common::HashMap<uint32, Range> _map;

		template<class StrictWeakOrdering>
		void setupSequences(StrictWeakOrdering comp);
		Common::Span<const TValue> get(uint32 key, const char *name) const;
	};

	void loadScripts();
	void loadCharAnimSets();
	void loadChoices();
	void loadRooms();

	const Common::Path path;
	SequenceSet<ScriptLine> _scripts;
	TwoKeyDataSet<CharacterAnimationSet> _charAnimSets;
	SequenceSet<Choice> _choices;
	SimpleDataSet<Room> _rooms;
};

}

#endif // EDNA_DB_H

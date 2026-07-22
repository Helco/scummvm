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

#ifndef EDNA_UTIL_H
#define EDNA_UTIL_H

#include "common/rect.h"
#include "common/span.h"
#include "common/hashmap.h"

namespace Edna {

static constexpr const int kScreenWidth = 800;
static constexpr const int kScreenHeight = 600;

static constexpr const Common::Point kInvalidPoint = { -1, -1 };

enum class Direction {
	Up = 0,
	Down,
	Left,
	Right
};
bool parseDirection(const char *text, Direction &value);

enum class GameMode {
	None = -1,
	StartMenu = 0,
	EdnaStd,
	Harvey,
	EdnaGirl,
	ScriptOnClick,
	DragScript,
	Zen,
	MainMenu, // should be unused, there is no main menu mode
	Intro,
};
const char *gameModeToString(GameMode mode);

enum PlayerAction {
	None = 0,
	Look,
	Use,
	Take,
	Talk
};
bool parsePlayerAction(const char *text, PlayerAction &value);

enum class Font {
	EdnaFont = 0,
	HarveyFont,
	NscFontRot,
	NscFontGelb,
	NscFontOrange,
	NscFontGreygreen,
	NscFontBlau,
	NscFontGrau,
	NscFontHellgelb,
	NscFontLind,
	NscFontStahlblau,
	NscFontWeiss,
	TestFont,
	ActiveFont,
	InactiveFont,
	MenuFont,
	MenuFont2
};

using ScriptId = uint32;
using CharAnimSetId = uint32;
using ActionModeId = uint32; // not a DB record, might be an enum
using AnimationId = uint32;
using AnimationFrameId = uint32;
using ChoiceSetId = uint32;
using RoomId = uint32;
using RoomObjectId = uint32;
using RoomObjectDisplayId = uint32;
using RoomInteractionId = uint32;
using RoomExitId = uint32;
using WalkableAreaId = uint32;
using TimerId = uint32;
using ItemId = uint32;
using TopicId = uint32;
using NPCId = uint32;

class Timer {
	uint32 _curDelay = 0, _delay = 0;
	bool _active = false;
public:
	inline uint32 &delay() { return _delay; }
	inline bool active() const { return _active; }

	bool update();
	void reset();
	void toggle(bool active);
};

struct AnimationRange {
	uint32 _startFrame = 0, _endFrame = 0, _delay = 0;
	bool _loop = false;
};

using StringSpan = Common::Span<char>;
using StringView = Common::Span<const char>;

using TwoKey = Common::Pair<uint32, uint32>;
struct TwoKeyHash {
	inline uint operator()(const TwoKey &key) const {
		return (uint)(key.first ^ key.second);
	}
};
struct TwoKeyEqualTo {
	inline bool operator()(const TwoKey &a, const TwoKey &b) const {
		return a.first == b.first && a.second == b.second;
	}
};
int compare(const TwoKey &a, const TwoKey &b);
bool less(const TwoKey &a, const TwoKey &b);
template<class TValue>
using TwoKeyMap = Common::HashMap<TwoKey, TValue, TwoKeyHash, TwoKeyEqualTo>;

}

#endif // EDNA_UTIL_H

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

namespace Graphics {
	class Font;
}

namespace Edna {

static constexpr const int16 kScreenWidth = 800;
static constexpr const int16 kScreenHeight = 600;

static constexpr const Common::Point kInvalidPoint = { -1, -1 };

enum class Direction {
	None = -1,
	Up = 0,
	Down,
	Left,
	Right
};
bool parseDirection(const char *text, Direction &value);
const char *directionToString(Direction dir);

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

enum class PlayerAction : uint {
	None = 0,
	Look,
	Use,
	Pick,
	Talk,
	Walk
};
bool parsePlayerAction(const char *text, PlayerAction &value);
const char *playerActionToString(PlayerAction action);

enum class FontKind {
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
const char *fontKindToString(FontKind kind);

struct FontInfo {
	Graphics::Font
		*_fgFont = nullptr,
		*_bgFont = nullptr; ///< background font is without antialising
	uint32_t _color = 0; //< in BlendBlit format
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

	inline AnimationRange(uint32 startFrame = 0, uint32 endFrame = 0, uint32 delay = 0, bool loop = false)
		: _startFrame(startFrame), _endFrame(endFrame), _delay(delay), _loop(loop) {}

	inline bool isValid() const { return _startFrame < _endFrame; }
	inline bool operator==(const AnimationRange &o) const {
		return _startFrame == o._startFrame && _endFrame == o._endFrame &&
			_delay == o._delay && _loop == o._loop;
	}
	inline bool operator!=(const AnimationRange &o) const {
		return !(*this == o);
	}
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

struct GameTransition {
	RoomId _room = 0;
	Common::Point _walkIn;
	Direction _walkInDir = Direction::None;
	ScriptId _script = 0;
	uint32 _scriptLine = 0;

	inline bool isPending() const { return _room != 0; }
};

struct PlayerCommand {
	PlayerAction _action = PlayerAction::None;
	uint32 _target = 0; ///< This can be an object, an item, a topic or even a gui element (player action)
	ItemId _item = 0; ///< Only for Use there can be a second target which is always an item
	Common::Point _targetPos = kInvalidPoint;

	bool _isComplete; ///< this cannot be determined by the members alone, e.g. an item can usually not be used as-is

	bool operator==(const PlayerCommand &command) const;
	bool operator!=(const PlayerCommand &command) const;
};

}

#endif // EDNA_UTIL_H

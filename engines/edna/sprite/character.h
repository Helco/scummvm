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

#ifndef EDNA_CHARACTER_H
#define EDNA_CHARACTER_H

#include "edna/sprite/animation.h"
#include "edna/sprite/object.h"

#include "audio/mixer.h"

namespace Edna {

class Game;

class Character : public virtual SpatialObject, public virtual AnimatedSprite {
public:
	enum State : ActionModeId { // not using an enum class as we need the constants as integers often
		kWalking,
		kTalking,
		kThinking,
		kWaiting,
		kActing
	};

	Character(
		Game &game,
		Common::Point startPos,
		CharAnimSetId charAnimSetId,
		float hSpeed, float vSpeed,
		float baseYAtZeroScale, float baseYAtFullScale);

	inline State state() const { return _state; }
	inline ActionModeId actionMode() const { return _actionMode; }
	inline Direction &direction() { return _direction; }

	void update() override;
	// TODO: Maybe Character should override render to cache scaled images

	void wait(uint32 duration);
	void lookIn(Direction direction);
	void say(const char *text, const char *soundFile);
	void think(const char *text, const char *soundFile);
	void shutUp(); ///< original method name and it fits...
	void walkTo(Common::Point walkTo, Direction standbyDirection = Direction::None);

protected:
	void sayOrThink(const char *text, const char *soundFile, State newState);
	using AnimatedSprite::setAnimation;
	void setState(State state);
	void setAnimation();
	void setAnimation(ActionModeId actionMode);
	void initScaling();
	void updateScaling();
	Common::Point scaleBasePosToSpritePos(Common::Point basePos) const;
	void updateTalking();
	void updateWalking();
	void nextWalkPoint();
	const char *stateToString() const; ///< for debugging

	static constexpr const uint32 kStateCount = 5;
	Game &_game;
	Timer _stateTimer;
	State _state = kWaiting;	
	ActionModeId _actionMode = kWaiting; ///< actionMode is usually state but can differ for special animations
	Direction _direction = Direction::Left;
	CharacterAnimationSet _charAnimSet;

	Audio::SoundHandle _talkSound;
	uint32 _talkTimeLeft = 0; ///< only used if speech is off
	Sprite *_talkText = nullptr;
	FontKind _talkFont = {};
	FontKind _thinkFont = {};

	const float _baseYAtZeroScale = 0, _baseYAtFullScale = 1;
	Common::Point _baseOffset;
	float _scaleFactor = 0;

	const float _hSpeed = 0, _vSpeed = 0;
	float _cSpeed = 0;
	Direction _standbyDirection = Direction::None;
	Common::Array<Common::Point> _walkPoints; ///< as unscaled base positions
	Common::Point _walkTarget; ///< as sprite position
};

}

#endif // EDNA_CHARACTER_H

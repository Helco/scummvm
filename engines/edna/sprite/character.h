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

namespace Edna {

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

protected:
	using AnimatedSprite::setAnimation;
	void setAnimation();
	void setAnimation(ActionModeId actionMode);
	void initScaling();
	void updateScaling();

	static constexpr const uint32 kStateCount = 5;
	Timer _stateTimer;
	State _state = kWaiting;	
	ActionModeId _actionMode = kWaiting; ///< actionMode is usually state but can differ for special animations
	Direction _direction = Direction::Left;
	CharacterAnimationSet _charAnimSet;

	const float _hSpeed = 0, _vSpeed = 0;
	const float _baseYAtZeroScale = 0, _baseYAtFullScale = 1;
	Common::Point _baseSize;
	float _scaleFactor = 0;
	float _cSpeed = 0;
};

}

#endif // EDNA_CHARACTER_H

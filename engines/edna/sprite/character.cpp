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

#include "edna/sprite/character.h"

using namespace Common;

namespace Edna {

Character::Character(
	CharAnimSetId charAnimSetId,
	float hSpeed, float vSpeed,
	float baseYAtZeroScale, float baseYAtFullScale)
	: _charAnimSet(charAnimSetId)
	, _hSpeed(hSpeed), _vSpeed(vSpeed)
	, _baseYAtZeroScale(baseYAtZeroScale), _baseYAtFullScale(baseYAtFullScale) {
	
	setTextures(_charAnimSet.textures());
	initScaling();
}

void Character::update() {
	AnimatedSprite::update();

	if (_stateTimer.update()) {
		_stateTimer.toggle(false);
		_state = kWaiting;
		setAnimation(kWaiting);
	}

	updateScaling();
}

void Character::wait(uint32 duration) {
	assert(_state == kWaiting); // this might not hold true
	_state = kActing;
	_stateTimer.delay() = duration;
	_stateTimer.toggle(true);
}

void Character::lookIn(Direction direction) {
	_direction = direction;
	_state = kWaiting;
	setAnimation(kWaiting);
}

void Character::setAnimation() {
	setAnimation(_actionMode);
}

void Character::setAnimation(ActionModeId actionMode) {
	_actionMode = actionMode;
	setAnimation(_charAnimSet.get(actionMode, _direction));
}

void Character::initScaling() {
	// TODO: This might differ between NPCs and Player -_-
	const Point size = this->size();
	_scaleFactor = 1.0f / (_baseYAtFullScale - _baseYAtZeroScale - size.y);

	const Point baseOffset(size.x / 2, size.y);
	float initScale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		initScale = (pos().y - _baseYAtZeroScale) / (_baseYAtFullScale - _baseYAtZeroScale);
	pos() -= Point(size.x / 2, size.y) * initScale;
	updateScaling();
}

void Character::updateScaling() {
	float scale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		scale = MAX(0.001f, ABS(pos().y - _baseYAtZeroScale) * _scaleFactor);
	size() = _baseSize * scale;
}

}

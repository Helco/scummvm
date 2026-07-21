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

#include "edna/db.h"
#include "edna/sprite/player.h"

namespace Edna {

void Player::update() {
	AnimatedSprite::update();

	// TODO: Handle walking, talking, thinking states

	if (_stateTimer.update()) {
		_stateTimer.toggle(false);
		_state = State::Waiting;
		// TODO: Reset animation
	}
}

int Player::basePosX() const {
	return pos().x + size().x / 2;
}

int Player::basePosY() const {
	return pos().x + size().y;
}

int Player::basePosY(int x) const {
	(void)x;
	return pos().x + size().y;
}

void Player::wait(uint32 duration) {
	assert(_state == State::Waiting); // this might not hold true
	_state = State::Acting;
	_stateTimer.delay() = duration;
	_stateTimer.toggle(true);
}

}

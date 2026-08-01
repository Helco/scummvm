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

#include "edna/input.h"

using namespace Common;

namespace Edna {

void Input::nextFrame() {
	_wasMouseLeftPressed = false;
	_wasMouseRightPressed = false;
	_wasMouseLeftReleased = false;
	_wasMouseRightReleased = false;
}

bool Input::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case EVENT_LBUTTONDOWN:
		_wasMouseLeftPressed = true;
		_isMouseLeftPressed = true;
		return true;
	case EVENT_LBUTTONUP:
		_wasMouseLeftReleased = true;
		_isMouseLeftPressed = false;
		return true;
	case EVENT_RBUTTONDOWN:
		_wasMouseRightPressed = true;
		_isMouseRightPressed = true;
		return true;
	case EVENT_RBUTTONUP:
		_wasMouseRightReleased = true;
		_isMouseRightPressed = false;
		return true;
	case EVENT_MOUSEMOVE:
		_mousePos = event.mouse;
		return true;
	default:
		return false;
	}
}

}

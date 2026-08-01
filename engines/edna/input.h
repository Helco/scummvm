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

#ifndef EDNA_INPUT_H
#define EDNA_INPUT_H

#include "edna/util.h"

#include "common/events.h"

namespace Edna {

class Input {
public:
	void nextFrame();
	bool handleEvent(const Common::Event &event);

	inline Common::Point mousePos() const { return _mousePos; }
	inline bool wasMouseLeftPressed() const { return _wasMouseLeftPressed; }
	inline bool wasMouseRightPressed() const { return _wasMouseRightPressed; }
	inline bool wasMouseLeftReleased() const { return _wasMouseLeftReleased; }
	inline bool wasMouseRightReleased() const { return _wasMouseRightReleased; }
	inline bool isMosueLeftPressed() const { return _isMouseLeftPressed; }
	inline bool isMouseRightPressed() const { return _isMouseRightPressed; }

private:
	bool _wasMouseLeftPressed = false,
		_wasMouseRightPressed = false,
		_wasMouseLeftReleased = false,
		_wasMouseRightReleased = false,
		_isMouseLeftPressed = false,
		_isMouseRightPressed = false;
	Common::Point _mousePos;
};

}

#endif // EDNA_INPUT_H

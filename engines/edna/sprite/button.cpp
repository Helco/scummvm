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

#include "edna/edna.h"
#include "edna/sprite/button.h"

using namespace Common;

namespace Edna {

Button::Button(uint32 id, Point pos, const String &path) {
	this->id() = id;
	this->pos() = pos;
	_normal = g_engine->renderer().loadTexture((path + ".png").c_str());
	_hovered = g_engine->renderer().loadTexture((path + "_a.png").c_str());
	_pressed = g_engine->renderer().loadTexture((path + "_p.png").c_str());
	setTexture(_normal);
}

void Button::update() {
	setTexture(_normal);
}

const char *Button::displayName() const {
	return _displayName;
}

void Button::setDisplayName(const char *name) {
	assert(name != nullptr);
	_displayName = name;
}

void Button::setHovered() {
	setTexture(_hovered);
}

void Button::setPressed() {
	setTexture(_pressed);
}

}

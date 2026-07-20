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
#include "edna/edna.h"
#include "edna/sprite/object.h"

#include "gui/debugger.h"

using namespace Common;

namespace Edna {

void RoomObject::toggle(bool isActive) {
	Sprite::toggle(isActive);
	if (id() == 0)
		return;

	auto dbObject = g_engine->db().roomObject(id());
	if (dbObject._active != isActive) {
		dbObject._active = isActive;
		// TODO: Save modified object back
	}
}

void SpatialObject::setBasePos(Point basePos) {
	pos() = basePos - Point(size().x / 2, size().y);
}

VisualObject::VisualObject(Point baseLineStart, Point baseLineEnd)
	: _baseLineStart(baseLineStart)
	, _baseLineEnd(baseLineEnd) { }

void VisualObject::debugPrint() {
	g_engine->getDebugger()->debugPrintf("Visual\n");
}

int VisualObject::basePosX() const {
	return pos().x + size().x / 2;
}

int VisualObject::basePosY() const {
	return pos().x + size().y;
}

int VisualObject::basePosY(int x) const {
	auto delta = _baseLineEnd - _baseLineStart;
	float ratio = delta.y / (float)delta.x;
	return (int)(_baseLineStart.y + ratio * (x - _baseLineStart.x));
}

}

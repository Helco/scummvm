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

InteractableRoomObject::InteractableRoomObject(RoomInteractionId interactionId)
	: _interactionId(interactionId) {}

const char *InteractableRoomObject::displayName() const {
	return g_engine->db().roomInteraction(_interactionId)._name;
}

PlayerAction InteractableRoomObject::defaultAction() const {
	return g_engine->db().roomInteraction(_interactionId)._defaultAction;
}

ScriptId InteractableRoomObject::scriptFor(PlayerAction action) const {
	return g_engine->db().roomInteraction(_interactionId).scriptFor(action);
}

Point InteractableRoomObject::interactionPos() const {
	const auto interaction = g_engine->db().roomInteraction(_interactionId);
	return { (int16)interaction._walkToX, (int16)interaction._walkToY };
}

Direction InteractableRoomObject::interactionDir() const {
	return g_engine->db().roomInteraction(_interactionId)._lookDirection;
}

VisualObject::VisualObject(Point baseLineStart, Point baseLineEnd)
	: _baseLineStart(baseLineStart)
	, _baseLineEnd(baseLineEnd) { }

void VisualObject::debugPrint() {
	AnimatedSprite::debugPrint("Visual");
}

int32 VisualObject::basePosX() const {
	return pos().x + size().x / 2;
}

int32 VisualObject::basePosY() const {
	return pos().x + size().y;
}

int32 VisualObject::basePosY(int x) const {
	auto delta = _baseLineEnd - _baseLineStart;
	float ratio = delta.y / (float)delta.x;
	return (int)(_baseLineStart.y + ratio * (x - _baseLineStart.x));
}

VisualInteractableRoomObject::VisualInteractableRoomObject(
	RoomInteractionId interactionId,
	Point baseLineStart, Point baseLineEnd)
	: VisualObject(baseLineStart, baseLineEnd)
	, InteractableRoomObject(interactionId) {}

void VisualInteractableRoomObject::debugPrint() {
	AnimatedSprite::debugPrint("VisualInteractable");
}

}

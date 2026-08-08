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

RoomInteractable::RoomInteractable(RoomInteractionId interactionId)
	: _interactionId(interactionId) {}


const char *RoomInteractable::displayName() const {
	return g_engine->db().roomInteraction(_interactionId)._name;
}

PlayerAction RoomInteractable::defaultAction() const {
	return g_engine->db().roomInteraction(_interactionId)._defaultAction;
}

ScriptId RoomInteractable::scriptFor(PlayerAction action) const {
	return g_engine->db().roomInteraction(_interactionId).scriptFor(action);
}

Point RoomInteractable::interactionPos() const {
	return g_engine->db().roomInteraction(_interactionId)._walkTo;
}

Direction RoomInteractable::interactionDir() const {
	return g_engine->db().roomInteraction(_interactionId)._lookDirection;
}

GameObject::GameObject() {}

GameObject::GameObject(Point baseLineStart, Point baseLineEnd)
	: _baseLineStart(baseLineStart)
	, _baseLineEnd(baseLineEnd) {}

int32 GameObject::basePosX() const {
	return pos().x + size().x / 2;
}

int32 GameObject::basePosY() const {
	return pos().y + size().y;
}

int32 GameObject::basePosY(int x) const {
	auto delta = _baseLineEnd - _baseLineStart;
	if (delta.x == 0)
		return basePosY(); // point-spatiality
	float ratio = delta.y / (float)delta.x;
	return (int)(_baseLineStart.y + ratio * (x - _baseLineStart.x));
}

void GameObject::setBasePos(Point p) {
	pos() = p - Point(size().x / 2, size().y);
}

void RoomObject::debugPrint() {
	AnimatedSprite::debugPrint("Visual");
}

void RoomObject::toggle(bool isActive) {
	Sprite::toggle(isActive);
	if (id() != 0)
		g_engine->db().toggleRoomObject(id(), isActive);
}

InteractableRoomObject::InteractableRoomObject(RoomInteractionId roiId)
	: GameObject()
	, RoomInteractable(roiId) {}

InteractableRoomObject::InteractableRoomObject(RoomInteractionId roiId, Point baseLineStart, Point baseLineEnd)
	: GameObject(baseLineStart, baseLineEnd)
	, RoomInteractable(roiId) {}

const char *InteractableRoomObject::displayName() const {
	return RoomInteractable::displayName();
}

void InteractableRoomObject::debugPrint() {
	AnimatedSprite::debugPrint("Interactable");
}

RoomExit::RoomExit(RoomInteractionId roiId, RoomExitId exitId)
	: InteractableRoomObject(roiId)
	, _exitId(exitId) {}

RoomExit::RoomExit(RoomInteractionId roiId, RoomExitId exitId, Point baseLineStart, Point baseLineEnd)
	: InteractableRoomObject(roiId, baseLineStart, baseLineEnd)
	, _exitId(exitId) {}

void RoomExit::debugPrint() {
	AnimatedSprite::debugPrint("Exit");
}

Item::Item(ItemId itemId) {
	id() = itemId;
	reloadImage();
}

void Item::reloadImage() {
	const String path = g_engine->db().item(id())._icon.get();
	setTextures({ path + ".png", path + "_a.png" });
	setFrame(0);
}

void Item::setHovered() {
	setFrame(1);
}

void Item::update() {
	GameObject::update();
	setFrame(0);
}

void Item::debugPrint() {
	g_engine->getDebugger()->debugPrintf("Item \"%s\"\n", displayName());
}

const char *Item::displayName() const {
	return g_engine->db().item(id())._name;
}

PlayerAction Item::defaultAction() const {
	return g_engine->db().item(id())._defaultAction;
}

ScriptId Item::scriptFor(PlayerAction action) const {
	return g_engine->db().item(id()).scriptFor(action);
}

Point Item::interactionPos() const {
	return kInvalidPoint;
}

Direction Item::interactionDir() const {
	return Direction::None;
}

}

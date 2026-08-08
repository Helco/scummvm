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

#ifndef EDNA_OBJECT_H
#define EDNA_OBJECT_H

#include "edna/sprite/sprite.h"

namespace Edna {

class IInteractable {
public:
	virtual PlayerAction defaultAction() const = 0;
	virtual ScriptId scriptFor(PlayerAction action) const = 0;
	virtual Common::Point interactionPos() const = 0;
	virtual Direction interactionDir() const = 0;
};

class RoomInteractable : public IInteractable {
public:
	RoomInteractable(RoomInteractionId interactionId);

	inline RoomInteractionId interactionId() const { return _interactionId; }
	const char *displayName() const; ///< this is not an override
	PlayerAction defaultAction() const override;
	ScriptId scriptFor(PlayerAction action) const override;
	Common::Point interactionPos() const override;
	Direction interactionDir() const override;

private:
	const RoomInteractionId _interactionId;
};

class GameObject : public AnimatedSprite {
public:
	GameObject(); // point-spatiality
	GameObject(Common::Point baseLineStart, Common::Point baseLineEnd); // line-spatiality

	int32 basePosX() const;
	int32 basePosY() const;
	int32 basePosY(int32 x) const;
	void setBasePos(Common::Point pos);

private:
	const Common::Point _baseLineStart, _baseLineEnd;
};

// The ID of a room object corresponds to its database object
class RoomObject : public GameObject {
public:
	using GameObject::GameObject;

	void debugPrint() override;
	void toggle(bool isActive) override;
};

class InteractableRoomObject : public GameObject, public RoomInteractable {
public:
	InteractableRoomObject(RoomInteractionId roiId);
	InteractableRoomObject(RoomInteractionId roiId, Common::Point baseLineStart, Common::Point baseLineEnd);

	const char *displayName() const override;
	void debugPrint() override;
};

class RoomExit final : public InteractableRoomObject {
public:
	RoomExit(RoomInteractionId roiId, RoomExitId exitId);
	RoomExit(RoomInteractionId roiId, RoomExitId exitId, Common::Point baseLineStart, Common::Point baseLineEnd);

	inline RoomExitId exitId() const { return _exitId; }
	void debugPrint() override;

private:
	const RoomExitId _exitId;
};

class Item final : public GameObject, public IInteractable {
public:
	Item(ItemId itemId);

	void reloadImage();
	void setHovered();
	void update() override;
	void debugPrint() override;

	const char *displayName() const override;
	PlayerAction defaultAction() const override;
	ScriptId scriptFor(PlayerAction action) const override;
	Common::Point interactionPos() const override;
	Direction interactionDir() const override;
};

}

#endif // EDNA_OBJECT_H

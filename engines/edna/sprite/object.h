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

class RoomObject : virtual public Sprite {
public:
	void toggle(bool isActive) override;
};

class SpatialObject : virtual public Sprite { // e.g. a Player is not a RoomObject
public:
	virtual int32 basePosX() const = 0;
	virtual int32 basePosY() const = 0;
	virtual int32 basePosY(int x) const = 0;

	void setBasePos(Common::Point pos);
};

class IInteractableObject : virtual public RoomObject {
public:
	virtual PlayerAction defaultAction() const = 0;
	virtual ScriptId scriptFor(PlayerAction action) const = 0;
	virtual Common::Point interactionPos() const = 0;
	virtual Direction interactionDir() const = 0;
};

class InteractableRoomObject : virtual public IInteractableObject {
public:
	InteractableRoomObject(RoomInteractionId interactionId);

	inline RoomInteractionId interactionId() const { return _interactionId; }
	PlayerAction defaultAction() const override;
	ScriptId scriptFor(PlayerAction action) const override;
	Common::Point interactionPos() const override;
	Direction interactionDir() const override;

private:
	const RoomInteractionId _interactionId;
};

class VisualObject : public virtual AnimatedSprite, public virtual SpatialObject {
public:
	VisualObject(Common::Point baseLineStart, Common::Point baseLineEnd);

	void debugPrint() override;
	int32 basePosX() const override;
	int32 basePosY() const override;
	int32 basePosY(int x) const override;

private:
	const Common::Point _baseLineStart, _baseLineEnd;
};

}

#endif // EDNA_OBJECT_H

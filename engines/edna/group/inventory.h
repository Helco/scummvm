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

#ifndef EDNA_INVENTORY_H
#define EDNA_INVENTORY_H

#include "edna/group/group.h"
#include "edna/sprite/button.h"

namespace Edna {

class Item;

class Inventory final : public Group {
public:
	Inventory();

	enum class State {
		Closed,
		Opening,
		Open,
		Locked
	};
	inline State state() const { return _state; }
	inline bool isClosed() const { return _state == State::Closed || _state == State::Locked; }
	
	void update() override;
	void updateSelection(Sprite *selection);
	bool updatePressed(Sprite *selection);
	void onItemsChanged();

private:
	void close();
	void toggleAllItems(bool active);
	void updateItems();

	static constexpr uint kFirstItemI = 5;
	Button _buttonLock, _buttonUnlock, _buttonUp, _buttonDown;
	AnimatedSprite _frame;
	State _state = State::Closed;
	uint _scroll = 0;
};

}

#endif // EDNA_INVENTORY_H

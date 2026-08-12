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

#ifndef EDNA_EDNASTD_H
#define EDNA_EDNASTD_H

#include "edna/game/ednagame.h"
#include "edna/group/inventory.h"

namespace Edna {

class EdnaStd : public EdnaGame {
public:
	EdnaStd(Common::ScopedPtr<GameBase> &myPtr, const GameTransition &transition);

	void initGroups() override;
	void update() override;
	void triggerInventoryUpdate() override;

private:
	bool isItem(Sprite *selection) const;

	Sprite *findSelection();
	void onMouseLeftPressed(Sprite *&selection);
	void onMouseLeftReleased(Sprite *selection);
	void onMouseRightPressed(Sprite *selection);

	StdInventory _inventory;
};

}

#endif // EDNA_EDNASTD_H

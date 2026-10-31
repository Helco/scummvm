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

#ifndef EDNA_SAVEMENU_H
#define EDNA_SAVEMENU_H

#include "edna/group/group.h"
#include "edna/sprite/button.h"
#include "edna/sprite/text.h"

namespace Edna {

class SaveMenu : public Group {
public:
	void open();
	void update() override;

protected:
	static constexpr int kSlotColCount = 3;
	static constexpr int kSlotCount = kSlotColCount * kSlotColCount;

	SaveMenu(bool isForLoading, Common::Point gridPos);
	void addRemainingSprites();
	void changeSelection(uint32 slotI);

	Button
		_btnConfirm,
		_btnDelete,
		_btnCancel;
	Sprite _slotImages[kSlotCount];
	Text _infoText;
	Common::ScopedPtr<Button> _btnSlots[kSlotCount];
	bool _isAvailable[kSlotCount] = {};
	int _selectedI = -1, _infoJokeIndex = 0;
	const TexturePtr _emptySlotImage;
	const bool _isForLoading;
};

class InGameSaveMenu : public SaveMenu {
public:
	InGameSaveMenu(bool isForLoading);
};

class StartSaveMenu : public SaveMenu {
public:
	StartSaveMenu();

private:
	Sprite _infoTextBg;
};

}

#endif // EDNA_SAVEMENU_H

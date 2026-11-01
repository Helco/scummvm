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

#ifndef EDNA_PAUSEMENU_H
#define EDNA_PAUSEMENU_H

#include "edna/group/group.h"
#include "edna/sprite/button.h"

#include "graphics/managed_surface.h"

namespace Edna {

class GameBase;
class PauseOptionsMenu;
class PauseSaveMenu;

class PauseMenu : public Group {
public:
	PauseMenu();

	void open();
	void update() override;

private:
	template<class TSubMenu>
	void openSubmenu(Common::ScopedPtr<TSubMenu> &group, Button &button);
	void toggleButtons(bool enable);

	Game *_lastGame = nullptr;

	Common::ScopedPtr<PauseSaveMenu> _saveGroup;
	Common::ScopedPtr<PauseSaveMenu> _loadGroup;
	Common::ScopedPtr<PauseOptionsMenu> _optionsGroup;
	Common::ScopedPtr<Group> _achievementGroup;

	Sprite _background;
	Button
		_btnAchievements,
		_btnOptions,
		_btnLoad,
		_btnSave,
		_btnContinue,
		_btnExit;

	Graphics::ManagedSurface
		_backgroundOverlay,
		_backgroundMenu,
		_backgroundTemp;
};

}

#endif // EDNA_PAUSEMENU_H

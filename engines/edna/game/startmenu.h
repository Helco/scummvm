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

#ifndef EDNA_STARTMENU_H
#define EDNA_STARTMENU_H

#include "edna/game/game.h"
#include "edna/sprite/button.h"

namespace Edna {

class IRenderedText;
class StartOptionsMenu;

class StartMenu : public GameBase {
public:
	StartMenu(Common::ScopedPtr<GameBase> &myPtr);

	void update() override;
	void render() override;
	void triggerMusicToggle() override;

private:
	template<class TSubMenu>
	void openSubMenu(Common::ScopedPtr<TSubMenu> &group, Button &button);
	void toggleButtons(bool active);
	void startNewGame();

	Common::ScopedPtr<Group> _loadGroup;
	Common::ScopedPtr<StartOptionsMenu> _optionsGroup;
	Common::ScopedPtr<Group> _achievementGroup;
	Common::ScopedPtr<IRenderedText> _versionText;

	Group _group;
	Sprite _background;
	Button
		_btnContinue,
		_btnAchievements,
		_btnNewGame,
		_btnOptions,
		_btnLoad,
		_btnExit;
};

}

#endif // EDNA_STARTMENU_H

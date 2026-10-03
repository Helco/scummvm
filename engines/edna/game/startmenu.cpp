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

#include "edna/assetcache.h"
#include "edna/db.h"
#include "edna/edna.h"
#include "edna/game/startmenu.h"
#include "edna/graphics.h"
#include "edna/group/optionsmenu.h"
#include "edna/input.h"

#include "base/version.h"
#include "graphics/cursorman.h"

using namespace Common;

namespace Edna {

static constexpr const RoomId StartMenuRoomId = 1;

StartMenu::StartMenu(Common::ScopedPtr<GameBase> &myPtr)
	: GameBase(myPtr, GameMode::StartMenu)
	, _group("StartMenu")
	, _background("Background")
	, _btnContinue(1, { 20, 240 }, String::format("gui/hauptmenue/%s/b_spielen", g_engine->language()))
	, _btnAchievements(2, { 20, 270 }, String::format("gui/hauptmenue/%s/b_erfolge", g_engine->language()))
	, _btnNewGame(3, { 20, 300 }, String::format("gui/hauptmenue/%s/b_neu", g_engine->language()))
	, _btnOptions(4, { 20, 330 }, String::format("gui/hauptmenue/%s/b_optionen", g_engine->language()))
	, _btnLoad(5, { 20, 360 }, String::format("gui/hauptmenue/%s/b_laden", g_engine->language()))
	, _btnExit(6, { 20, 390 }, String::format("gui/hauptmenue/%s/b_beenden", g_engine->language())) {

	const auto dbRoom = g_engine->db().room(StartMenuRoomId);
	g_engine->playMusic(dbRoom._music);
	_background.setTexture(dbRoom._background);

	_group.add(&_background, DisposeAfterUse::NO);
	_group.add(&_btnContinue, DisposeAfterUse::NO);
	_group.add(&_btnAchievements, DisposeAfterUse::NO);
	_group.add(&_btnNewGame, DisposeAfterUse::NO);
	_group.add(&_btnOptions, DisposeAfterUse::NO);
	_group.add(&_btnLoad, DisposeAfterUse::NO);
	_group.add(&_btnExit, DisposeAfterUse::NO);
	add(&_group, DisposeAfterUse::NO);

	_versionText.reset(g_engine->renderer().createText(
		g_engine->assets().font(Edna::FontKind::MenuFont),
		String::format("Edna %s (ScummVM %s)", g_engine->versionExtra(), gScummVMVersion).c_str()
	));

	CursorMan.showMouse(true);
}

void StartMenu::update() {
	GameBase::update();

	if ((_loadGroup != nullptr && _loadGroup->active()) ||
		(_optionsGroup != nullptr && _optionsGroup->active()) ||
		(_achievementGroup != nullptr && _achievementGroup->active()))
		return;

	toggleButtons(true);
	Button *selection = dynamic_cast<Button *>(_group.checkClick(g_engine->input().mousePos()));
	if (selection != nullptr) {
		if (g_engine->input().isMouseLeftPressed()) {
			selection->setPressed();
			if (selection == &_btnContinue) {
				warning("Unimplemented startmenu button: continue");
			} else if (selection == &_btnAchievements) {
				warning("Unimplemented startmenu button: achievements");
			} else if (selection == &_btnNewGame) {
				startNewGame();
			} else if (selection == &_btnOptions) {
				openSubMenu(_optionsGroup, _btnOptions);
			} else if (selection == &_btnLoad) {
				warning("Unimplemented startmenu button: load");
			} else if (selection == &_btnExit) {
				g_engine->quitGame();
			}
		}
		else
			selection->setHovered();
	}
}

void StartMenu::render() {
	GameBase::render();

	const Point textPos = Point(g_system->getWidth(), g_system->getHeight()) - _versionText->size();
	g_engine->renderer().text(_versionText.get(), textPos);
}

void StartMenu::triggerMusicToggle() {
	if (g_engine->config().music()) {
		const auto room = g_engine->db().room(StartMenuRoomId);
		g_engine->playMusic(room._music);
	}
	else
		g_engine->stopMusic();
}

template<class TSubMenu>
void StartMenu::openSubMenu(Common::ScopedPtr<TSubMenu> &group, Button &button) {
	if (group == nullptr) {
		group.reset(new TSubMenu());
		add(group.get(), DisposeAfterUse::NO);
	}
	group->active() = true;

	toggleButtons(false);
	button.toggle(true);
	button.isDisabled() = true;
}

void StartMenu::toggleButtons(bool active) {
	_btnContinue.toggle(active);
	_btnAchievements.toggle(active);
	_btnNewGame.toggle(active);
	_btnOptions.toggle(active);
	_btnLoad.toggle(active);
	_btnExit.toggle(active);

	_btnLoad.isDisabled() = false;
	_btnAchievements.isDisabled() = false;
	_btnOptions.isDisabled() = false;
}

void StartMenu::startNewGame() {
	// TODO: Trigger dejavu achievement
	g_engine->next()._room = 2;
	g_engine->next()._walkIn = Point(400, 520);
	g_engine->next()._walkInDir = Direction::Left;
	g_engine->db().resetOverlay();
}

}

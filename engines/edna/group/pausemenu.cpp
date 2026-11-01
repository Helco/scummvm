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

#include "edna/edna.h"
#include "edna/game/game.h"
#include "edna/group/optionsmenu.h"
#include "edna/group/pausemenu.h"
#include "edna/group/savemenu.h"
#include "edna/input.h"

using namespace Common;
using namespace Graphics;

namespace Edna {

static constexpr const Point kAchievementBtnPos(120, 150);
static constexpr const Point kOptionsBtnPos(120, 180);
static constexpr const Point kLoadBtnPos(120, 210);
static constexpr const Point kSaveBtnPos(120, 240);

static void convertToGrayscale(ManagedSurface &surface) {
	assert(!surface.empty());
	assert(surface.format == BlendBlit::getSupportedPixelFormat());
	uint8 r, g, b;

	for (int y = 0; y < surface.h; y++) {
		union {
			uint32 *pixel;
			uint8 *components;
		};
		pixel = (uint32 *)surface.getBasePtr(0, y);
		for (int x = 0; x < surface.w; x++, pixel++) {
			surface.format.colorToRGB(*pixel, r, g, b);
			uint8 gray = (uint8)CLIP(0.29f * r + 0.58f * g + 0.11f * b, 0.0f, 255.0f);
			*pixel = surface.format.ARGBToColor(255, gray, gray, gray);
		}
	}
}

PauseMenu::PauseMenu()
	: Group("PauseMenu")
	, _background("Background")
	, _btnAchievements(1, kAchievementBtnPos, String::format("gui/hauptmenue/%s/b_erfolge", g_engine->language()))
	, _btnOptions(2, kOptionsBtnPos, String::format("gui/hauptmenue/%s/b_optionen", g_engine->language()))
	, _btnLoad(3, kLoadBtnPos, String::format("gui/hauptmenue/%s/b_laden", g_engine->language()))
	, _btnSave(4, kSaveBtnPos, String::format("gui/hauptmenue/%s/b_speichern", g_engine->language()))
	, _btnContinue(5, { 120, 390 }, String::format("gui/hauptmenue/%s/b_spielen", g_engine->language()))
	, _btnExit(6, { 120, 420 }, String::format("gui/hauptmenue/%s/b_beenden", g_engine->language())) {

	add(&_background, DisposeAfterUse::NO);
	add(&_btnAchievements, DisposeAfterUse::NO);
	add(&_btnOptions, DisposeAfterUse::NO);
	add(&_btnLoad, DisposeAfterUse::NO);
	add(&_btnSave, DisposeAfterUse::NO);
	add(&_btnContinue, DisposeAfterUse::NO);
	add(&_btnExit, DisposeAfterUse::NO);

	_backgroundOverlay = loadPNG("gui/hauptmenue/filterscreen.png");
	_backgroundOverlay.convertToInPlace(BlendBlit::getSupportedPixelFormat());
	_backgroundMenu = loadPNG("gui/hauptmenue/bg_menu.png");
	_backgroundMenu.convertToInPlace(BlendBlit::getSupportedPixelFormat());
	_backgroundTemp.create(g_system->getWidth(), g_system->getHeight(), BlendBlit::getSupportedPixelFormat());

	_saveGroup.reset(new PauseSaveMenu(false));
	_loadGroup.reset(new PauseSaveMenu(true));
	_optionsGroup.reset(new PauseOptionsMenu());
	_achievementGroup.reset(new Group("Achievements"));
}

void PauseMenu::open() {
	if (_lastGame != &g_engine->game()) {
		_lastGame = dynamic_cast<Game *>(&g_engine->game());
		assert(_lastGame != nullptr);
		_lastGame->add(this, DisposeAfterUse::NO);
		_lastGame->add(_saveGroup.get(), DisposeAfterUse::NO);
		_lastGame->add(_loadGroup.get(), DisposeAfterUse::NO);
		_lastGame->add(_optionsGroup.get(), DisposeAfterUse::NO);
		_lastGame->add(_achievementGroup.get(), DisposeAfterUse::NO);
	}
	assert(_lastGame != nullptr);

	// the background consists of a grayscale screenshot,
	// an overlay image and a menu frame image blended on top
	auto &renderer = g_engine->renderer();
	renderer.beginOffScreen();
	_lastGame->screenshotRender();
	renderer.end();
	_backgroundTemp.simpleBlitFrom(renderer.offScreen());
	convertToGrayscale(_backgroundTemp);
	_backgroundOverlay.blendBlitTo(_backgroundTemp);
	_backgroundMenu.blendBlitTo(_backgroundTemp, 135, 150);
	_background.setTexture(renderer.loadTexture(_backgroundTemp));

	toggleButtons(true);
	_saveGroup->active() = false;
	_loadGroup->active() = false;
	_optionsGroup->active() = false;
	_achievementGroup->active() = false;
	active() = true;
}

void PauseMenu::update() {
	Group::update();
	if (!active() ||
		_saveGroup->active() ||
		_loadGroup->active() ||
		_optionsGroup->active() ||
		_achievementGroup->active())
		return;

	toggleButtons(true);
	Button *selection = dynamic_cast<Button *>(checkClick(g_engine->input().mousePos()));
	if (selection != nullptr)
		selection->setHovered();
	if (selection != nullptr && g_engine->input().wasMouseLeftPressed()) {
		selection->setPressed();
		if (selection == &_btnAchievements) {
			warning("Unimplemented pausemenu button: achievements");
		} else if (selection == &_btnOptions) {
			openSubmenu(_optionsGroup, _btnOptions);
		} else if (selection == &_btnLoad) {
			openSubmenu(_loadGroup, _btnLoad);
		} else if (selection == &_btnSave) {
			openSubmenu(_saveGroup, _btnSave);
		} else if (selection == &_btnContinue) {
			active() = false;
		} else if (selection == &_btnExit) {
			warning("Unimplemented pausemenu button: exit");
		}
	}
}

template<class TSubMenu>
void PauseMenu::openSubmenu(ScopedPtr<TSubMenu> &group, Button &button) {
	group->open();

	toggleButtons(false);
	button.toggle(true);
	button.isDisabled() = true;
	button.pos() = kAchievementBtnPos;
}

void PauseMenu::toggleButtons(bool enable) {
	_btnAchievements.toggle(enable);
	_btnAchievements.isDisabled() = false;
	_btnAchievements.pos() = kAchievementBtnPos;
	_btnOptions.toggle(enable);
	_btnOptions.isDisabled() = false;
	_btnOptions.pos() = kOptionsBtnPos;
	_btnLoad.toggle(enable);
	_btnLoad.isDisabled() = false;
	_btnLoad.pos() = kLoadBtnPos;
	_btnSave.toggle(enable);
	_btnSave.isDisabled() = false;
	_btnSave.pos() = kSaveBtnPos;
}

}

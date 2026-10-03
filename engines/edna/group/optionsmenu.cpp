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
#include "edna/edna.h"
#include "edna/game/game.h"
#include "edna/graphics.h"
#include "edna/group/optionsmenu.h"
#include "edna/input.h"
#include "edna/translation.h"

using namespace Common;

namespace Edna {

OptionsMenu::OptionsMenu()
	: Group("Options")
	, _background("Background")
	, _textSpeedBg("TextSpeedBg")
	, _textSpeedSlider("TextSpeedSlider")
	, _btnConfirm(1, Point(), "gui/hauptmenue/b_okay")
	, _btnCancel(2, Point(), "gui/hauptmenue/b_chancel")
	, _textMusic(Point(), FontKind::MenuFont, g_engine->translation().toggleMusic(), kTextNone)
	, _textSound(Point(), FontKind::MenuFont, g_engine->translation().toggleSound(), kTextNone)
	, _textSubtitles(Point(), FontKind::MenuFont, g_engine->translation().toggleSubtitles(), kTextNone)
	, _textTextSpeed(Point(), FontKind::MenuFont, g_engine->translation().textSpeed(), kTextNone) {

	_textSpeedBg.immutable() = true;
	_textSpeedSlider.immutable() = true;
	_textTextSpeed.immutable() = true;

	_background.setTexture("gui/hauptmenue/bg-optionsmenu.png");
	_textSpeedBg.setTexture("gui/hauptmenue/reglerleiste.png");
	_textSpeedSlider.setTexture("gui/hauptmenue/regler-normal.png");
	std::initializer_list<String> checkBoxTextures = {
		"gui/hauptmenue/b-unchecked.png",
		"gui/hauptmenue/b-checked.png"
	};
	_checkMusic.setTextures(checkBoxTextures);
	_checkSound.setTextures(checkBoxTextures);
	_checkSubtitles.setTextures(checkBoxTextures);
	_infoText.reset(g_engine->renderer().createText(g_engine->assets().font(FontKind::MenuFont), ""));

	add(&_background, DisposeAfterUse::NO);
	add(&_textSpeedBg, DisposeAfterUse::NO);
	add(&_textSpeedSlider, DisposeAfterUse::NO);
	add(&_checkMusic, DisposeAfterUse::NO);
	add(&_checkSound, DisposeAfterUse::NO);
	add(&_checkSubtitles, DisposeAfterUse::NO);
	add(&_btnConfirm, DisposeAfterUse::NO);
	add(&_btnCancel, DisposeAfterUse::NO);
	add(&_textMusic, DisposeAfterUse::NO);
	add(&_textSound, DisposeAfterUse::NO);
	add(&_textSubtitles, DisposeAfterUse::NO);
	add(&_textTextSpeed, DisposeAfterUse::NO);

	_previousConfig = g_engine->config();
}

InGameOptionsMenu::InGameOptionsMenu() : OptionsMenu() {
	_background.pos() = Point(225, 150);
	_textSpeedBg.pos() = Point(240, 400);
	_textSpeedSlider.pos() = Point(238, 390);
	_checkMusic.pos() = Point(238, 210);
	_checkSubtitles.pos() = Point(238, 250);
	_checkSound.pos() = Point(238, 290);
	_btnConfirm.pos() = Point(580, 450);
	_btnCancel.pos() = Point(552, 450);
	_textMusic.pos() = Point(280, 210);
	_textSubtitles.pos() = Point(280, 250);
	_textSound.pos() = Point(280, 290);
	_textTextSpeed.pos() = Point(280, 350);
	_infoTextPos = Point(235, 453);
	_sliderMinX = 238;
	_sliderMaxX = 540;

	toggleChecks();
}

StartOptionsMenu::StartOptionsMenu() : OptionsMenu() {
	_background.pos() = Point(120, 230);
	_textSpeedBg.pos() = Point(150, 450);
	_textSpeedSlider.pos() = Point(148, 440);
	_checkMusic.pos() = Point(148, 260);
	_checkSubtitles.pos() = Point(148, 300);
	_checkSound.pos() = Point(148, 340);
	_btnConfirm.pos() = Point(440, 490);
	_btnCancel.pos() = Point(470, 490);
	_textMusic.pos() = Point(190, 260);
	_textSubtitles.pos() = Point(190, 300);
	_textSound.pos() = Point(190, 340);
	_textTextSpeed.pos() = Point(190, 400);
	_infoTextPos = Point(135, 203);
	_sliderMinX = 148;
	_sliderMaxX = 450;

	toggleChecks();
}

void OptionsMenu::update() {
	if (!active())
		return;
	Group::update();

	if (_isMovingSlider) {
		if (g_engine->input().isMouseLeftPressed())
			_isMovingSlider = false;
		else {
			int newSlider = CLIP(g_engine->input().mousePos().x, _sliderMinX, _sliderMaxX);
			uint8 newSpeed = (uint8)CLIP(newSlider * 255 / (_sliderMaxX - _sliderMinX), 0, 255);
			g_engine->config().subtitleSpeed() = newSpeed;
			_textSpeedSlider.pos().x = newSlider - 14;
		}
	}

	const Rect infoTextRect(_infoTextPos, _infoTextPos + _infoText->size());
	const bool clicked = g_engine->input().wasMouseLeftPressed();
	Sprite *selection = checkClick(g_engine->input().mousePos());
	if (selection == &_btnConfirm) {
		if (clicked) {
			active() = false;
			g_engine->config().saveToScummVM();
		} else
			_btnConfirm.setHovered();
	} else if (selection == &_btnCancel) {
		if (clicked) {
			g_engine->config() = _previousConfig;
			active() = false;
			g_engine->config().saveToScummVM();
			g_engine->game().triggerMusicToggle();
		} else
			_btnCancel.setHovered();
	} else if (selection == &_checkMusic && clicked) {
		g_engine->config().music() = !g_engine->config().music();
		toggleChecks();
		g_engine->game().triggerMusicToggle();
	} else if (selection == &_checkSound && clicked) {
		g_engine->config().speech() = !g_engine->config().speech();
		toggleChecks();
	} else if (selection == &_checkSubtitles && clicked) {
		g_engine->config().subtitles() = !g_engine->config().subtitles();
		toggleChecks();
	} else if (clicked && infoTextRect.contains(g_engine->input().mousePos())) {
		U32String joke = g_engine->translation().infoJoke(_infoJokeIndex++);
		if (joke.empty()) {
			_infoJokeIndex = 0;
			joke = g_engine->translation().infoJoke(_infoJokeIndex);
		}
		assert(!joke.empty());
		_infoText->setText(joke);
	} else if (selection == &_textSpeedSlider && clicked)
		_isMovingSlider = true;
}

void OptionsMenu::render() {
	if (!active())
		return;
	Group::render();

	g_engine->renderer().text(_infoText.get(), _infoTextPos);
}

void OptionsMenu::open() {
	_previousConfig = g_engine->config();
	toggleChecks();
	active() = true;
}

void OptionsMenu::toggleChecks() {
	Config &config = g_engine->config();

	if (!config.speech() && !config.subtitles())
		config.subtitles() = true;

	_checkMusic.setFrame(config.music() ? 1 : 0);
	_checkSubtitles.setFrame(config.subtitles() ? 1 : 0);
	_checkSound.setFrame(config.speech() ? 1 : 0);
	_textSpeedBg.toggle(!config.speech());
	_textSpeedSlider.toggle(!config.speech());
	_textTextSpeed.toggle(!config.speech());

	_textSpeedSlider.pos().x = (int16)(_sliderMinX + (_sliderMaxX - _sliderMinX) * config.subtitleSpeed() / 255 - 14);
}

}

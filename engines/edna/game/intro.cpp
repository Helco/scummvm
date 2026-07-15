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
#include "edna/game/intro.h"

using namespace Common;

namespace Edna {

Intro::Intro()
	: GameBase(GameMode::Intro)
	, _group("intro") {

	_daedalic.setTexture("bildfolgen/intro/daedalic.png");
	_splash.setTexture((String("gui/littleSplash_") + g_engine->language() + ".png").c_str());
	_splash.pos() = { 250, 100 };
	_splash.active() = false;
	_splash.immutable() = true;
	_group.add(&_daedalic, DisposeAfterUse::NO);
	_group.add(&_splash, DisposeAfterUse::NO);
	add(&_group, DisposeAfterUse::NO);

	_music = g_engine->playMusic("music/jingle-spiritual", false);
	_lastFrame = g_engine->getMillis();
}

Intro::~Intro() {
	g_system->getMixer()->stopHandle(_music);
}

void Intro::update() {
	GameBase::update();

	uint32 elapsed = g_engine->getMillis() - _lastFrame;
	_lastFrame = g_engine->getMillis();

	switch (_stage) {
	case Stage::FadeIn:
		_fadeAlpha -= elapsed / 1000.0f;
		if (_fadeAlpha < 0.01f) {
			_fadeAlpha = 0.01f;
			_stage = Stage::FadeOut;
		}
		break;
	case Stage::FadeOut:
		_fadeAlpha += elapsed / 3500.0f;
		if (_fadeAlpha > 0.99f) {
			_fadeAlpha = 0.99f;
			_daedalic.active() = false;
			_splash.active() = true;
			_stage = Stage::Harvey;
			g_engine->nextRoom() = 1; // startmenu
		}
		break;
	case Stage::Harvey:
		break;
	default:
		assert("Invalid intro stage");
		break;
	}
}

void Intro::render() {
	if (_stage == Stage::Harvey) {
		g_engine->renderer().rect({ 0, 0, kScreenWidth, kScreenHeight }, 255, 255, 255, 255);
		GameBase::render();
	} else {
		GameBase::render();
		uint8 alpha = (uint8)CLIP(_fadeAlpha * 255.0f, 0.0f, 255.0f);
		g_engine->renderer().rect({ 0, 0, kScreenWidth, kScreenHeight }, 255, 255, 255, alpha);
	}
}

}

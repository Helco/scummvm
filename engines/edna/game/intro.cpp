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

#include "graphics/surface.h"
#include "image/png.h"

using namespace Common;

namespace Edna {

Intro::Intro(ScopedPtr<GameBase> &myPtr, bool withHarvey)
	: GameBase(myPtr, GameMode::Intro)
	, _withHarvey(withHarvey)
	, _group("intro") {

	_daedalic.setTexture("bildfolgen/intro/daedalic.png");
	_group.add(&_daedalic, DisposeAfterUse::NO);
	add(&_group, DisposeAfterUse::NO);
	_music = g_engine->playMusic("music/jingle-spiritual", false);

	if (_withHarvey) {
		_splash.setTexture((String("gui/littleSplash_") + g_engine->language() + ".png").c_str());
		_splash.pos() = { 250, 100 };
		_splash.toggle(false);
		_splash.immutable() = true;
		_group.add(&_splash, DisposeAfterUse::NO);

		// this is the only sprite sheet animation, so we load it manually here
		File file;
		Image::PNGDecoder decoder;
		if (!file.open("gui/harvey_walk_50x100x7.png") || !decoder.loadStream(file))
			error("Could not open harvey intro animation");
		const Graphics::Surface &surface = *decoder.getSurface();
		const int CellCount = 7;
		int cellWidth = surface.w / CellCount;
		TexturePtr cells[CellCount];
		for (int i = 0; i < CellCount; i++) {
			cells[i] = g_engine->renderer().loadTexture(
				surface.getSubArea(Rect(i * cellWidth, 0, (i + 1) * cellWidth, surface.h)));
		}

		_harvey.setTextures(TextureSpan(cells, CellCount));
		_harvey.setAnimation(AnimationRange{ 0, CellCount - 1,  200, true });
		_harvey.pos() = { 375, 300 };
		_harvey.toggle(false);
		_harvey.immutable() = true;
		_group.add(&_harvey, DisposeAfterUse::NO);
	}
}

Intro::~Intro() {
	g_system->getMixer()->stopHandle(_music);
}

void Intro::update() {
	GameBase::update();

	switch (_stage) {
	case Stage::FadeIn:
		_fadeAlpha -= g_engine->getElapsed() / 1000.0f;
		if (_fadeAlpha < 0.01f) {
			_fadeAlpha = 0.01f;
			_stage = Stage::FadeOut;
		}
		break;
	case Stage::FadeOut:
		_fadeAlpha += g_engine->getElapsed() / 3500.0f;
		if (_fadeAlpha > 0.99f) {
			_fadeAlpha = 0.99f;

			if (_withHarvey) {
				_stage = Stage::Harvey;
				_daedalic.toggle(false);
				_splash.toggle(true);
				_harvey.toggle(true);
			}
			else
				g_engine->next()._room = 1; // startmenu
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

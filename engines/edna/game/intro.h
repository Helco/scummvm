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

#ifndef EDNA_INTRO_H
#define EDNA_INTRO_H

#include "edna/game/game.h"
#include "edna/sprite/sprite.h"

#include "audio/mixer.h"

namespace Edna {

class Intro final : public GameBase {
public:
	Intro(Common::ScopedPtr<GameBase> &myPtr, bool withHarvey);
	virtual ~Intro();

	void update() override;
	void render() override;

private:
	enum class Stage {
		FadeIn,
		FadeOut,
		Harvey
	};

	const bool _withHarvey = false;
	Stage _stage = Stage::FadeIn;
	Group _group;
	Sprite _daedalic, _splash;
	AnimatedSprite _harvey;
	float _fadeAlpha = 1.0f;
	Audio::SoundHandle _music;
};

}

#endif // EDNA_INTRO_H

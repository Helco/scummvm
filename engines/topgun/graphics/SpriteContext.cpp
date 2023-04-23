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

#include "common/system.h"
#include "graphics/wincursor.h"
#include "graphics/palette.h"

#include "topgun/topgun.h"
#include "topgun/graphics/SpriteContext.h"

namespace TopGun {

static uint32 roundingFractionMul(uint32 v, uint32 num, uint32 denom) {
	return (denom / 2 + v * num) / denom;
}

SpriteContext::SpriteContext(TopGunEngine *engine) :
	_engine(engine),
	_screen(new Graphics::Screen()),
	_busyWinCursor(Graphics::makeBusyWinCursor()),
	_currentPalette{0},
	_targetPalette{0} {

}

void SpriteContext::SetPaletteFromResourceFile() {
	auto& resFilePalette = _engine->getResourceFile()->_palette;
	const size_t maxCopyBytes = kHighSystemColors - kLowSystemColors - _engine->getResourceFile()->_maxTransColors;
	const size_t copyBytes = MIN(maxCopyBytes, (size_t)resFilePalette.size());
	Common::copy(resFilePalette.begin(), resFilePalette.begin() + copyBytes, _targetPalette + kLowSystemColors * 3);

	g_system->getPaletteManager()->setPalette(_targetPalette, 0, kPaletteSize);
	FadePalette(1, 1, kLowSystemColors, _engine->getResourceFile()->_maxFadeColors);
}

void SpriteContext::FadePalette(uint32 t, uint32 maxT, byte colorOffset, byte colorCount) {
	assert(colorOffset + colorCount <= kPaletteSize);

	if (maxT == 0)
		maxT = 1;
	t = MIN(t, maxT);

	size_t byteOffset = colorOffset * 3;
	size_t byteCount = colorCount * 3;
	for (size_t i = byteOffset; i < byteOffset + byteCount; i++)
		_currentPalette[i] = roundingFractionMul(_targetPalette[i], t, maxT);
	g_system->getPaletteManager()->setPalette(_currentPalette + byteOffset, colorOffset, colorCount);
}

}

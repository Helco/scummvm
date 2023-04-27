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

#ifndef TOPGUN_GRAPHICS_H
#define TOPGUN_GRAPHICS_H

#include "common/scummsys.h"
#include "common/ptr.h"
#include "graphics/screen.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "graphics/font.h"

#include "Sprite.h"

using Common::Array;
using Common::ScopedPtr;
using Common::SharedPtr;

namespace TopGun {

class TopGunEngine;
class SpriteContext {
	friend class Sprite;
public:
	static constexpr int32 kSystemBusyCursor = 0;
	static constexpr int32 kCursorCount = 9;
	static constexpr uint32 kCursorGroupResourceID = 1001;
	static constexpr size_t kPaletteSize = 256;
	static constexpr size_t kLowSystemColors = 10;
	static constexpr size_t kHighSystemColors = 246;

	SpriteContext(TopGunEngine *engine);
	~SpriteContext();

	Common::SharedPtr<Sprite> createSprite(uint32 index);
	void setPaletteFromResourceFile();
	void fadePalette(uint32 t, uint32 maxT, byte colorOffset, byte colorCount);
	void setCursor(int32 id);
	SharedPtr<Graphics::Font> loadFont(const Common::String &name, int32 height);

	inline TopGunEngine *getEngine() {
		return _engine;
	}

private:
	void loadCursors();
	uint32 getSpriteIndex(const Sprite *sprite) const;
	void resortSprite(const Sprite *sprite);
	void setPaletteFromTopMostSprite(Common::ReadStream &stream, uint32 colorCount);

private:
	TopGunEngine *_engine;

	Common::Array<Common::SharedPtr<Sprite> > _sprites; // intentionally not using SortedArray
	uint32 _nestedSpriteLoops;
	uint32 _curSpriteIndex;

	ScopedPtr<Graphics::Screen> _screen;
	Array<Graphics::Cursor *> _cursors; // unowned pointers
	ScopedPtr<Graphics::Cursor> _busyCursor;
	ScopedPtr<Graphics::Cursor> _defaultCursor;
	Array<Graphics::WinCursorGroup *> _cursorGroups;

	Common::Array<Common::SharedPtr<Graphics::Font> > _fonts;
	Common::Array<Common::Pair<Common::String, int> > _fontTopGunNames;

	byte _targetPalette[kPaletteSize * 3];
	byte _currentPalette[kPaletteSize * 3];
};

}

#endif

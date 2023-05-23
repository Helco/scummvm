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

#include "Bitmap.h"
#include "Sprite.h"

using Common::Array;
using Common::ScopedPtr;
using Common::SharedPtr;

namespace TopGun {

enum class BackgroundAnimation {
	kNone
};

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
	static constexpr size_t kMaxSceneColors = kHighSystemColors - kLowSystemColors;

	SpriteContext(TopGunEngine *engine);
	~SpriteContext();

	void render();
	void animate();

	void resetScene();
	SharedPtr<Sprite> createSprite(uint32 index);
	SharedPtr<Graphics::Font> loadFont(const Common::String &name, int32 height);
	void setPaletteFromResourceFile();
	void fadePalette(uint32 t, uint32 maxT, byte colorOffset, byte colorCount);
	void setCursor(int32 id);
	void setBackground(
		uint32 highResBitmap,
		uint32 lowResBitmap,
		BackgroundAnimation animation = BackgroundAnimation::kNone,
		int32 animArg1 = 0,
		int32 animArg2 = 0);
	void setBackground(byte r, byte g, byte b);
	void setBackground(byte color);

	inline TopGunEngine *getEngine() {
		return _engine;
	}

private:
	void loadCursors();
	uint32 getSpriteIndex(const Sprite *sprite) const;
	void resortSprite(const Sprite *sprite);
	void setPaletteFromTopMostSprite(Common::ReadStream &stream, uint32 colorCount);
	void resetBackgroundBounds();
	void clipScrollBox();
	byte getNearestSceneColor(byte r, byte g, byte b);

private:
	TopGunEngine *_engine;

	Point _scrollPos, _backgroundOffset;
	Rect _screenBounds;
	Rect _backgroundBounds;
	Rect _fullBackgroundBounds;
	Rect _clipBox;
	Rect _clippedScrollBox;
	Rect _scrollBox;
	ScopedPtr<Graphics::Screen> _screen;

	Array<SharedPtr<Sprite> > _sprites; // intentionally not using SortedArray
	uint32 _nestedSpriteLoops;
	uint32 _curSpriteIndex;

	Array<Graphics::Cursor *> _cursors; // unowned pointers
	ScopedPtr<Graphics::Cursor> _busyCursor;
	ScopedPtr<Graphics::Cursor> _defaultCursor;
	Array<Graphics::WinCursorGroup *> _cursorGroups;

	Array<SharedPtr<Graphics::Font> > _fonts;
	Array<Common::Pair<Common::String, int> > _fontTopGunNames;

	SharedPtr<Bitmap> _bitmapBackground;
	byte _colorBackground;

	byte _targetPalette[kPaletteSize * 3];
	byte _currentPalette[kPaletteSize * 3];
	byte _sceneColorCount;
};

}

#endif

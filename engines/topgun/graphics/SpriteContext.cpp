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
#include "common/formats/winexe.h"
#include "graphics/wincursor.h"
#include "graphics/palette.h"
#include "graphics/fonts/ttf.h"

#include "topgun/topgun.h"
#include "topgun/graphics/SpriteContext.h"

namespace TopGun {

static uint32 roundingFractionMul(uint32 v, uint32 num, uint32 denom) {
	return (denom / 2 + v * num) / denom;
}

SpriteContext::SpriteContext(TopGunEngine *engine) :
	_engine(engine),
	_screen(new Graphics::Screen()),
	_currentPalette{0},
	_targetPalette{0},
	_nestedSpriteLoops(0),
	_curSpriteIndex(0) {

	loadCursors();
	_screenBounds.left = (_screen->w - 1) / -2;
	_screenBounds.top = (_screen->h - 1) / -2;
	_screenBounds.right = _screenBounds.left + _screen->w;
	_screenBounds.bottom = _screenBounds.top + _screen->h;
}

SpriteContext::~SpriteContext() {
	for (auto cursorGroup : _cursorGroups)
		delete cursorGroup;
}

void SpriteContext::render() {
	_screen->clear(_colorBackground);

	if (_bitmapBackground != nullptr) {
		Rect srcRect(_backgroundBounds.width(), _backgroundBounds.height());
		srcRect.translate(_backgroundOffset.x, _backgroundOffset.y);
		srcRect.translate(-_scrollPos.x, -_scrollPos.y);
		Point dstPos(-_screenBounds.left, -_screenBounds.top);
		_screen->blitFrom(*_bitmapBackground->getSurface(), srcRect, dstPos);
	}

	for (auto sprite : _sprites)
		sprite->render(_backgroundBounds);

	/*auto r = Rect(_screen->w / 256, _screen->h / 10);
	for (int i = 0; i < 256; i++)
	{
		_screen->fillRect(r, i);
		r.translate(r.width(), 0);
	}*/

	_screen->update();
}

void SpriteContext::animate() {
	_nestedSpriteLoops++;

	for (; _curSpriteIndex < _sprites.size(); _curSpriteIndex++) {
		_sprites[_curSpriteIndex]->animate();
	}
	_curSpriteIndex = 0;

	_nestedSpriteLoops--;
}

void SpriteContext::resetScene() {
	// TODO: reset scrollBox
	setClipBox();
	setBackground(0);

	_sprites.clear();
	_fonts.clear();
	_fontTopGunNames.clear();
}

Common::SharedPtr<Sprite> SpriteContext::createSprite(uint32 index) {
	auto sprite = Common::SharedPtr<Sprite>(new Sprite(this, index));
	_sprites.push_back(sprite); // shortly invalidating order, to be fixed shortly during load
	return sprite;
}

void SpriteContext::setPaletteFromResourceFile() {
	auto& resFilePalette = _engine->getResourceFile()->_palette;
	const size_t maxCopyBytes = kHighSystemColors - kLowSystemColors - _engine->getResourceFile()->_maxTransColors;
	const size_t copyBytes = MIN(maxCopyBytes, (size_t)resFilePalette.size());
	Common::copy(resFilePalette.begin(), resFilePalette.begin() + copyBytes, _targetPalette + kLowSystemColors * 3);
	_sceneColorCount = resFilePalette.size() / 3;

	g_system->getPaletteManager()->setPalette(_targetPalette, 0, kPaletteSize);
	fadePalette(1, 1, kLowSystemColors, _engine->getResourceFile()->_maxFadeColors);
}

void SpriteContext::fadePalette(uint32 t, uint32 maxT, byte colorOffset, byte colorCount) {
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

void SpriteContext::loadCursors() {
	_cursorGroups.reserve(kCursorCount - 2);
	_cursors.reserve(kCursorCount);
	_busyCursor.reset(Graphics::makeBusyWinCursor());
	_defaultCursor.reset(Graphics::makeDefaultWinCursor());
	_cursors.push_back(_busyCursor.get());
	_cursors.push_back(_defaultCursor.get());

	auto winResource = Common::WinResources::createFromEXE("RTLIB32.DLL");
	if (winResource == nullptr)
		error("Could not open RTLIB32.DLL to load cursor groups");
	for (size_t i = 0; i < kCursorCount - 2; i++) {
		_cursorGroups.push_back(Graphics::WinCursorGroup::createCursorGroup(winResource, kCursorGroupResourceID + i));
		_cursors.push_back(_cursorGroups[i]->cursors[0].cursor);
	}
	delete winResource;

	// FIXME: Why does CursorMan have a replaceCursor(Cursor*) method but not a pushCursor(Cursor*) one?
	CursorMan.pushCursor(NULL, 0, 0, 0, 0, 0);
}

void SpriteContext::setCursor(int32 id) {
	CursorMan.replaceCursor(_cursors[id]);
}

uint32 SpriteContext::getSpriteIndex(const Sprite *sprite) const {
	for (uint32 i = 0; i < _sprites.size(); i++) {
		if (_sprites[i].get() == sprite)
			return i;
	}
	assert(false);
	return UINT32_MAX;
}

void SpriteContext::resortSprite(const Sprite *sprite) {
	uint32 oldIndex = getSpriteIndex(sprite);
	auto spritePtr = _sprites.remove_at(oldIndex);

	if (_nestedSpriteLoops && oldIndex > _curSpriteIndex)
		_curSpriteIndex--;

	uint32 newIndex = _sprites.size();
	if (_engine->getTopMostSprite().get() != sprite) {
		for (; newIndex > 0 && _sprites[newIndex - 1]->_level > sprite->_level; newIndex--) {
		}
	}
	_sprites.insert_at(newIndex, spritePtr);
}

void SpriteContext::setPaletteFromTopMostSprite(Common::ReadStream &stream, uint32 colorCount) {
	const auto colorOffset = kHighSystemColors - _engine->getResourceFile()->_maxTransColors;
	assert(colorOffset + colorCount < kPaletteSize);
	for (uint32 i = 0; i < colorCount; i++) {
		_currentPalette[(colorOffset + i) * 3 + 0] = stream.readByte();
		_currentPalette[(colorOffset + i) * 3 + 1] = stream.readByte();
		_currentPalette[(colorOffset + i) * 3 + 2] = stream.readByte();
		stream.readByte();
	}
	g_system->getPaletteManager()->setPalette(_currentPalette + colorOffset * 3, colorOffset, colorCount);
}

struct FontMapping {
	const char *topgunName;
	const char *scummName;
};

static const FontMapping fontMappings[] = {
	{ "Arial", "LiberationSans-Regular.ttf" },
	{ "Times Roman", "LiberationSerif-Regular.ttf"},
	{ nullptr, nullptr }
};

SharedPtr<Graphics::Font> SpriteContext::loadFont(const Common::String &name, int32 height) {
	for (size_t i = 0; i < _fonts.size(); i++) {
		if (_fontTopGunNames[i].first == name && _fontTopGunNames[i].second == height)
			return _fonts[i];
	}

	const FontMapping *mapping = fontMappings;
	while (mapping->topgunName != nullptr && name != mapping->topgunName)
		mapping++;
	if (mapping->topgunName == nullptr)
		error("Unknown TopGun font %s", name.c_str());

	auto font = SharedPtr<Graphics::Font>(Graphics::loadTTFFontFromArchive(mapping->scummName, height));
	if (font == nullptr)
		error("Could not load ScummVM font %s", mapping->scummName);
	_fonts.push_back(font);
	_fontTopGunNames.push_back(Common::Pair<Common::String, int>(name, height));
	return font;
}

void SpriteContext::resetBackgroundBounds() {
	_scrollPos = Point(0, 0);
	_backgroundBounds.left = _backgroundBounds.top = 0;
	_backgroundBounds.right = _bitmapBackground == nullptr ? _screen->w : _bitmapBackground->getSurface()->w;
	_backgroundBounds.bottom = _bitmapBackground == nullptr ? _screen->h : _bitmapBackground->getSurface()->h;

	_backgroundOffset.x = (_backgroundBounds.right - 1) / 2;
	_backgroundOffset.y = (_backgroundBounds.bottom - 1) / 2;
	_backgroundBounds.translate(-_backgroundOffset.x, -_backgroundOffset.y);
	_fullBackgroundBounds = _backgroundBounds;

	if (_clipBox.right > _clipBox.left)
		_backgroundBounds.clip(_clipBox);
	_backgroundBounds.clip(_screenBounds);

	clipScrollBox();
}

void SpriteContext::clipScrollBox() {
	_clippedScrollBox = _screenBounds;
	if (_scrollBox.right > _scrollBox.left && _scrollBox.top < _scrollBox.bottom) {
		_clippedScrollBox.clip(_scrollBox);
		_clippedScrollBox.clip(_backgroundBounds);
	}

	// TODO: Add tile background handling here
	if (_bitmapBackground != nullptr)
		_clippedScrollBox.clip(_fullBackgroundBounds);
}

void SpriteContext::setClipBox(Rect rect) {
	_clipBox = rect;
	if (_clipBox.left < _clipBox.right && _clipBox.top < _clipBox.bottom) {
		_clipBox.right++;
		_clipBox.bottom++;
	}
	resetBackgroundBounds();
}

byte SpriteContext::getNearestSceneColor(byte r, byte g, byte b) {
	int minIndex = -1;
	int minScore = INT32_MAX;
	for (int i = kLowSystemColors; i < kLowSystemColors + _sceneColorCount; i++) {
		int curScore =
			ABS(_currentPalette[i * 3 + 0] - r) +
			ABS(_currentPalette[i * 3 + 1] - g) +
			ABS(_currentPalette[i * 3 + 2] - b);
		if (curScore >= minScore)
			continue;
		minIndex = i;
		curScore = i;
		if (curScore == 0)
			return i;
	}

	if (_sceneColorCount <= kMaxSceneColors - _engine->getResourceFile()->_maxTransColors) {
		minIndex = kLowSystemColors + _sceneColorCount++;
		_currentPalette[minIndex * 3 + 0] = r;
		_currentPalette[minIndex * 3 + 1] = g;
		_currentPalette[minIndex * 3 + 2] = b;
		g_system->getPaletteManager()->setPalette(_currentPalette + minIndex * 3, minIndex, 1);
	}
	return minIndex;
}

void SpriteContext::setBackground(byte r, byte g, byte b) {
	setBackground(getNearestSceneColor(r, g, b));
}

void SpriteContext::setBackground(byte color) {
	_bitmapBackground.reset();
	_colorBackground = color;
	resetBackgroundBounds();
}

void SpriteContext::setBackground(
	uint32 highResBitmap,
	uint32 lowResBitmap,
	BackgroundAnimation animation,
	int32 animArg1,
	int32 animArg2) {
	if (animation != BackgroundAnimation::kNone)
		warning("Background animations are not implemented yet");

	_colorBackground = 0;
	uint32 myResBitmap = highResBitmap; // TODO: Add low res handling
	switch (_engine->getResourceType(myResBitmap)) {
	case ResourceType::kBitmap:
		_bitmapBackground = _engine->loadResource<Bitmap>(myResBitmap);
		break;
	case ResourceType::kCell:
		error("Cell backgrounds are not implemented yet");
		break;
	default:
		error("Invalid background resource %d type %d", myResBitmap, _engine->getResourceType(myResBitmap));
		break;
	}

	resetBackgroundBounds();
}

}

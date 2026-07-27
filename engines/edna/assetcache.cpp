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
#include "edna/graphics.h"

#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

using namespace Common;

namespace Edna {

struct StaticFontInfo {
    constexpr StaticFontInfo(uint8 size, uint8 r, uint8 g, uint8 b)
        : _size(size), _r(r), _g(g), _b(b) {}
    uint8 _size = 0, _r = 0, _g = 0, _b = 0;
};
static constexpr const StaticFontInfo kStaticFonts[] = {
    { 18, 208, 141, 230 }, // EdnaFont
    { 18, 109, 213, 63 }, // HarveyFont
    { 18, 190, 111, 87 }, // NscFontRot
    { 18, 228, 192, 82 }, // NscFontGelb
    { 18, 255, 175, 103 }, // NscFontorange
    { 18, 164, 186, 120 }, // NscFontGreygreen
    { 18, 137, 205, 222 }, // NscFontBlau
    { 18, 205, 204, 196 }, // NscFontGrau
    { 18, 232, 227, 143 }, // NscFontHellgelb
    { 18, 156, 210, 152 }, // NscFontLind
    { 18, 144, 147, 196 }, // NscFontStahlblau
    { 18, 255, 255, 255 }, // NscFontWeiss
    { 18, 255, 255, 0 }, // TestFont
    { 18, 243, 239, 171 }, // ActiveFont
    { 18, 136, 136, 136 }, // InactiveFont
    { 14, 255, 255, 255 }, // MenuFont
    { 16, 255, 255, 255 } // MenuFont2
};

AssetCache::AssetCache() {
	_fgFont14.reset(loadFont(14, false));
	_bgFont14.reset(loadFont(14, true));
	_fgFont16.reset(loadFont(16, false));
	_bgFont16.reset(loadFont(16, true));
	_fgFont18.reset(loadFont(18, false));
	_bgFont18.reset(loadFont(18, true));
}

Graphics::Font *AssetCache::loadFont(int size, bool forBackground) {
	auto file = new File();
	if (!file->open("data/DejaVuSans.ttf")) {
		delete file;
		error("Could not open font file: %d %s", size, forBackground ? "bg" : "fg");
	}
	const auto sizeMode = Graphics::kTTFSizeModeCharacter;
	const auto renderMode = forBackground
		? Graphics::kTTFRenderModeMonochrome
		: Graphics::kTTFRenderModeNormalWithGridFitting;
	auto *font = Graphics::loadTTFFont(file, DisposeAfterUse::YES, size, sizeMode, 72, 72, renderMode, nullptr, true);
	if (font == nullptr)
		error("Could not load font: %d %s", size, forBackground ? "bg" : "fg");
	return font;
}

const FontInfo AssetCache::font(FontKind kind) const {
    int kindI = (int)kind;
    scumm_assert(kindI < ARRAYSIZE(kStaticFonts));
    const auto &staticInfo = kStaticFonts[kindI];

    auto format = Graphics::BlendBlit::getSupportedPixelFormat();
    FontInfo info;
    info._color = format.ARGBToColor(255, staticInfo._r, staticInfo._g, staticInfo._b);
    switch (staticInfo._size) {
    case 14:
        info._fgFont = _fgFont14.get();
        info._bgFont = _bgFont14.get();
        break;
    case 16:
        info._fgFont = _fgFont16.get();
        info._bgFont = _bgFont16.get();
        break;
    case 18:
        info._fgFont = _fgFont18.get();
        info._bgFont = _bgFont18.get();
        break;
    default:
        error("Unimplemented font size");
    }
    return info;
}

SharedPtr<ITexture> AssetCache::texture(const String &fileName) {
    SharedPtr<ITexture> texture;
    if (_textureCaches[0].tryGetVal(fileName, texture) ||
        _textureCaches[1].tryGetVal(fileName, texture))
        return texture;

    texture = g_engine->renderer().loadTexture(fileName.c_str());
    if (texture != nullptr)
        _textureCaches[_nextTextureCache][fileName] = texture;
    return texture;
}

void AssetCache::finishNextLoad() {
    _nextTextureCache = !_nextTextureCache;
    _textureCaches[_nextTextureCache].clear();
}

}

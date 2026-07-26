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

#include "edna/util.h"
#include "edna/graphics.h"

#include "common/system.h"
#include "engines/util.h"
#include "graphics/blit.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"
#include "image/png.h"

using namespace Common;

namespace Edna {

ITexture::~ITexture() { }

void ITexture::setDebugName(const char *name) {
	_debugName = name;
}

const char *ITexture::debugName() const {
	return _debugName.c_str();
}

IRenderedText::~IRenderedText() { }
IRenderer::~IRenderer() { }

TexturePtr IRenderer::loadTexture(const char *fileName) {
	File file;
	if (!file.open(fileName) && !file.open(Path(String(fileName) + ".png")))
		return nullptr;
	Image::PNGDecoder decoder;
	if (!decoder.loadStream(file))
		return nullptr;
	auto texture = loadTexture(*decoder.getSurface());
	if (texture != nullptr)
		texture->setDebugName(lastPathComponent(fileName, '/').c_str());
	return texture;
}

static Graphics::AlphaType getAlphaType(const Graphics::ManagedSurface &surface) {
	assert(surface.format.bytesPerPixel == 4); // this should be true for BlendBlit::getSupportedPixelFormat 
	Graphics::AlphaType type = Graphics::ALPHA_OPAQUE;
	byte *line = (byte*)surface.getPixels();
	for (int16 y = 0; y < surface.h; y++) {
		uint32 *pixel = (uint32 *)line;
		for (int16 x = 0; x < surface.w; x++, pixel++) {
			auto alpha = (*pixel >> surface.format.aShift) & 255;
			if (alpha == 0)
				type = Graphics::ALPHA_BINARY;
			else if (alpha != 255)
				return Graphics::ALPHA_FULL;
		}
		line += surface.pitch;
	}
	return type;
}

class SoftwareTexture final : public ITexture {
public:
	Graphics::ManagedSurface _surface;
	const Graphics::AlphaType _alphaType;

	SoftwareTexture(Graphics::ManagedSurface &&surface)
		: _surface(Common::move(surface))
		, _alphaType(getAlphaType(_surface)) {}

	Point size() const override {
		return Point(_surface.w, _surface.h);
	}

	bool alphaCheck(Point pos) const override {
		if (pos.x < 0 || pos.y < 0 || pos.x >= _surface.w || pos.y >= _surface.h)
			return false;
		uint32 pixel = _surface.getPixel(pos.x, pos.y);
		uint8 a, r, g, b;
		_surface.format.colorToARGB(pixel, a, r, g, b);
		return a == 255;
	}
};

class SoftwareRenderedText final : public IRenderedText {
public:
	static constexpr uint kBorderRadius = 3;

	Graphics::Font *_bgFont, *_fgFont;
	uint32 _color;
	Graphics::ManagedSurface _surface;
	U32String _text;

	SoftwareRenderedText(const FontInfo &fontInfo, const char *textBegin, const char *textEnd)
		: _bgFont(fontInfo._bgFont)
		, _fgFont(fontInfo._fgFont)
		, _color(fontInfo._color) {
		if (textBegin != nullptr)
			setText(textBegin, textEnd);
	}

	Common::Point size() const override {
		return Point(_surface.w, _surface.h);
	}

	void setColor(const FontInfo &info) override {
		assert(info._bgFont == _bgFont && info._fgFont == _fgFont);
		_color = info._color;
	}

	void setText(const char *textBegin, const char *textEnd) override {
		assert(textBegin != nullptr);
		assert(textEnd == nullptr || textBegin <= textEnd);
		if (textEnd == nullptr)
			textEnd = textBegin + strlen(textBegin);
		U32String newText(textBegin, textEnd, kUtf8);
		if (_text == newText)
			return;
		_text = newText;
		if (_text.empty()) {
			_surface.free();
			return;
		}

		// The original code would draw 5x5 times in bg color, then once again in fg color (26 string draws...)
		// Unfortunately I found no alternative that would achieve the same look
		Rect rect = _bgFont->getBoundingBox(_text);
		rect.grow(kBorderRadius * 2);
		Graphics::ManagedSurface tmpSurface(rect.width(), rect.height(), g_system->getScreenFormat());
		if (_surface.w == rect.width() && _surface.h == rect.height())
			_surface.clear();
		else
			_surface.create(rect.width(), rect.height(), g_system->getScreenFormat());

		const uint32 black = _surface.format.RGBToColor(0, 0, 0);
		for (int i = 1; i < 6; i++) {
			for (int j = 1; j < 6; j++)
				_bgFont->drawString(&_surface, _text, i, j, _surface.w - 2 * kBorderRadius, black, Graphics::kTextAlignCenter);
		}
		_fgFont->drawString(&_surface, _text, kBorderRadius, kBorderRadius, _surface.w - 2 * kBorderRadius, 0xffffffff, Graphics::kTextAlignCenter);
		// we render the string in white so we can multiply with a color without rerendering the text
		// the border is always black so is not affected by it
	}
};

class SoftwareRenderer final : public IRenderer {
	ScopedPtr<Graphics::Screen> _screen;
public:
	SoftwareRenderer() {
		const auto format = Graphics::BlendBlit::getSupportedPixelFormat();
		initGraphics(kScreenWidth, kScreenHeight, &format);
		_screen.reset(new Graphics::Screen());
	}

	TexturePtr loadTexture(const Graphics::Surface &surface) override {
		Graphics::ManagedSurface converted; // TODO: check whether any images are paletted and cannot be converted like this
		converted.convertFrom(surface, g_system->getScreenFormat());
		return TexturePtr(new SoftwareTexture(Common::move(converted)));
	}

	IRenderedText *createText(const FontInfo &fontInfo, const char *textBegin, const char *textEnd) override {
		return new SoftwareRenderedText(fontInfo, textBegin, textEnd);
	}

	void begin() override { }

	void end() override {
		_screen->markAllDirty();
		_screen->update();
	}

	void sprite(ITexture *textureRaw, Point pos, Point size = {}) override {
		auto texture = dynamic_cast<SoftwareTexture *>(textureRaw);
		assert(texture != nullptr);
		if (size == Point())
			size = texture->size();

		texture->_surface.blendBlitTo(
			*_screen,
			pos.x, pos.y,
			Graphics::FLIP_NONE,
			nullptr,
			0xffffffff, // colormod
			size.x, size.y,
			Graphics::BLEND_NORMAL,
			texture->_alphaType);
	}

	void text(IRenderedText *textRaw, Point pos) override {
		auto text = dynamic_cast<SoftwareRenderedText *>(textRaw);
		assert(text != nullptr);

		text->_surface.blendBlitTo(
			*_screen,
			pos.x, pos.y,
			Graphics::FLIP_NONE,
			nullptr,
			text->_color,
			-1, -1, // destination size
			Graphics::BLEND_NORMAL,
			Graphics::ALPHA_FULL);
	}

	void rect(Rect rect, uint8 r, uint8 g, uint8 b, uint8 a) override {
		rect.clip(kScreenWidth, kScreenHeight);
		if (rect.isEmpty())
			return;

		auto color = Graphics::BlendBlit::getSupportedPixelFormat().ARGBToColor(a, r, g, b);
		Graphics::BlendBlit::fill(
			(byte *)_screen->getBasePtr(rect.left, rect.top),
			_screen->pitch,
			rect.width(), rect.height(),
			color,
			Graphics::BLEND_NORMAL);
	}

	void debugPoints(Span<const Point> points, uint8 r, uint8 g, uint8 b) override {
		uint32 color = _screen->format.RGBToColor(r, g, b);
		auto bounds = _screen->getBounds();
		for (const auto &point : points) {
			if (bounds.contains(point))
				_screen->setPixel(point.x, point.y, color);
		}
	}
};

IRenderer *createSoftwareRenderer() {
	return new SoftwareRenderer();
}

}

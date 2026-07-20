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
#include "engines/util.h"
#include "graphics/blit.h"
#include "graphics/managed_surface.h"
#include "graphics/screen.h"
#include "image/png.h"

#include "edna/util.h"
#include "edna/graphics.h"

using namespace Common;

namespace Edna {

ITexture::~ITexture() { }
IRenderedText::~IRenderedText() { }
IRenderer::~IRenderer() { }

TexturePtr IRenderer::loadTexture(const char *fileName) {
	File file;
	if (!file.open(fileName))
		return nullptr;
	Image::PNGDecoder decoder;
	if (!decoder.loadStream(file))
		return nullptr;
	return loadTexture(*decoder.getSurface());
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
	Graphics::Font *_font;
	uint8 _r, _g, _b;
	Graphics::ManagedSurface _surface;
	U32String _text;

	SoftwareRenderedText(Graphics::Font *font, uint8 r, uint8 g, uint8 b)
		: _font(font), _r(r), _g(g), _b(b) {}

	virtual Common::Point size() const {
		return Point(_surface.w, _surface.h);
	}

	virtual void setText(const char *text) {
		U32String newText(text, kUtf8);
		if (_text == newText)
			return;
		_text = newText;
		if (_text.empty()) {
			_surface.free();
			return;
		}

		Rect rect = _font->getBoundingBox(_text);
		if (_surface.w != rect.width() || _surface.h != rect.height())
			_surface.create(rect.width(), rect.height(), g_system->getScreenFormat());
		uint32 color = _surface.format.RGBToColor(_r, _g, _b);
		_font->drawAlphaString(&_surface, _text, 0, 0, _surface.w, color);
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

	IRenderedText *createText(Graphics::Font *font, uint8 r, uint8 g, uint8 b) override {
		return new SoftwareRenderedText(font, r, g, b);
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

		uint32 color = text->_surface.format.ARGBToColor(
			255, text->_r, text->_g, text->_b);
		text->_surface.blendBlitTo(
			*_screen,
			pos.x, pos.y,
			Graphics::FLIP_NONE,
			nullptr,
			color,
			-1, -1, // destination size
			Graphics::BLEND_NORMAL,
			Graphics::ALPHA_FULL); // TODO: Check if we can reduce to ALPHA_BINARY
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

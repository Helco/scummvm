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

#ifndef EDNA_GRAPHICS_H
#define EDNA_GRAPHICS_H

#include "common/rect.h"
#include "common/span.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Edna {

class ITexture {
	Common::String _debugName;
public:
	virtual ~ITexture();

	virtual Common::Point size() const = 0;
	virtual bool alphaCheck(Common::Point pos) const = 0;

	void setDebugName(const char *name);
	const char *debugName() const;
};

using TexturePtr = Common::SharedPtr<ITexture>;
using TextureArray = Common::Array<TexturePtr>;
using TextureSpan = Common::Span<TexturePtr>;

class IRenderedText {
public:
	virtual ~IRenderedText();

	virtual Common::Point size() const = 0;
	virtual void setColor(const FontInfo &fontInfo) = 0; ///< font is not touched, ideally no rerendering should be done
	virtual void setText(const char *textBegin, const char *textEnd = nullptr) = 0;
};

class IRenderer {
public:
	virtual ~IRenderer();

	TexturePtr loadTexture(const char *fileName);
	virtual TexturePtr loadTexture(const Graphics::Surface &surface) = 0;
	virtual IRenderedText *createText(const FontInfo &fontInfo, const char *textBegin = nullptr, const char *textEnd = nullptr) = 0;

	virtual void begin() = 0;
	virtual void sprite(ITexture *texture, Common::Point pos, Common::Point size = {}) = 0;
	virtual void text(IRenderedText *text, Common::Point pos) = 0;
	virtual void rect(Common::Rect rect, uint8 r, uint8 g, uint8 b, uint8 a) = 0;
	virtual void end() = 0;

	virtual void debugPoints(Common::Span<const Common::Point> points, uint8 r, uint8 g, uint8 b) = 0;
	virtual void debugRect(Common::Rect rect, uint8 r, uint8 g, uint8 b) = 0;
	virtual void debugLine(Common::Point pos1, Common::Point pos2, uint8 r, uint8 g, uint8 b) = 0;
	virtual void debugText(Common::Point pos, const Common::String &text, uint8 r, uint8 g, uint8 b,
		Graphics::TextAlign align = Graphics::kTextAlignCenter) = 0;
};

IRenderer *createSoftwareRenderer();

}

#endif // EDNA_GRAPHICS_H

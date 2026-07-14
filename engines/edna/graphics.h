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

namespace Edna {

class ITexture {
public:
	virtual ~ITexture();

	virtual Common::Point size() const = 0;
	virtual bool alphaCheck(Common::Point pos) const = 0;
};

class IRenderedText {
public:
	virtual ~IRenderedText();

	virtual Common::Point size() const = 0;
	virtual void setText(const char *text) = 0;
};

class IRenderer {
public:
	virtual ~IRenderer();

	virtual ITexture *loadTexture(const char *fileName) = 0;
	virtual IRenderedText *createText(Graphics::Font *font, uint8 r, uint8 g, uint8 b) = 0;

	virtual void begin() = 0;
	virtual void sprite(ITexture *texture, Common::Point pos, Common::Point size = {}) = 0;
	virtual void text(IRenderedText *text, Common::Point pos) = 0;
	virtual void rect(Common::Rect rect, uint8 r, uint8 g, uint8 b, uint8 a) = 0;
	virtual void end() = 0;

	virtual void debugPoints(Common::Span<const Common::Point> points, uint8 r, uint8 g, uint8 b) = 0;
};

IRenderer *createSoftwareRenderer();

}

#endif // EDNA_GRAPHICS_H

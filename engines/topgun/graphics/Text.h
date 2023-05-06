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

#ifndef TOPGUN_TEXT_H
#define TOPGUN_TEXT_H

#include "graphics/fontman.h"
#include "topgun/Resource.h"

namespace TopGun {

class SpriteContext;
class Sprite;

enum class TextAlignment : byte {
	kCenter = 0,
	kRight = (1 << 0),
	kLeft = (1 << 1),
	kBottom = (1 << 2),
	kTop = (1 << 3)
};

class Text : public ISurfaceResource {
public:
	static constexpr ResourceType kResourceType = ResourceType::kText;

	Text(SpriteContext *spriteCtx, uint32 index);
	virtual ~Text() = default;

	virtual bool load(Common::Array<byte> &&data) override;

	void addSpriteReference(Common::WeakPtr<Sprite> sprite);
	void renderText();

	inline SpriteContext *getSpriteContext() {
		return _spriteCtx;
	}
	virtual Point getOffset() const override {
		return _offset;
	}
	virtual Graphics::Surface *getSurface() override {
		return _surface->surfacePtr();
	}

private:
	SpriteContext *_spriteCtx;

	Common::Array<Common::WeakPtr<Sprite> > _referencingSprites;
	Common::ScopedPtr<Graphics::ManagedSurface> _surface;
	Common::SharedPtr<Graphics::Font> _font;
	Common::String _value;
	Point _offset;
	Point _pos;
	Point _size;
	TextAlignment _alignment = TextAlignment::kCenter;
	byte _color = 0;
	bool _wrap = false;
	bool _isPassword = false;
	bool _showLastPasswordCharacter = false;
};

}

#endif // TOPGUN_TEXT_H

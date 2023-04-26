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

#ifndef TOPGUN_SPRITE_H
#define TOPGUN_SPRITE_H

#include "topgun/Resource.h"

namespace TopGun {

class SpriteContext;

class Sprite : public IResource {
	friend class SpriteContext;

	Sprite(SpriteContext *spriteContext, uint32 index);
public:
	static constexpr ResourceType kResourceType = ResourceType::kSprite;

	virtual ~Sprite() = default;

	virtual bool load(Common::Array<byte> &&data) override;

	void setLevel(int32 newLevel);
	void addCell(Common::SharedPtr<ISurfaceResource> resource);

	inline SpriteContext *getSpriteContext() {
		return _spriteCtx;
	}
	inline Point getPos() const {
		return _pos;
	}
	inline Rect getBounds() const {
		return _bounds;
	}

private:
	void setBoundsByCurrentCell();

private:
	SpriteContext *_spriteCtx;
	Common::Array<Common::SharedPtr<ISurfaceResource> > _cells;

	Point _pos;
	Point _scrollPos;
	Rect _bounds;

	bool _isScrollable;
	bool _animateCell;
	bool _animateCellsForward;
	bool _setToNextCellOnRepaint;
	uint32 _cellIndexStart, _cellIndexStop;
	uint32 _curCellIndex;
	int32 _level;
};

}

#endif // TOPGUN_SPRITE_H

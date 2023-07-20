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

#ifndef TOPGUN_CELL_H
#define TOPGUN_CELL_H

#include "graphics/surface.h"
#include "topgun/graphics/Bitmap.h"

namespace TopGun {

// A cell resource is basically just a reference to a Bitmap resource
// with a different offset

class Cell : public ISurfaceResource {
public:
	static constexpr ResourceType kResourceType = ResourceType::kCell;

	Cell(uint32 index);
	virtual ~Cell() = default;

	virtual bool load(Common::Array<byte> &&data) override;

	inline uint32 getInnerResourceIndex() const {
		return _bitmap->getResourceIndex();
	}

	virtual Point getOffset() const {
		return _offset;
	}

	virtual Graphics::Surface *getSurface() {
		return _bitmap->getSurface();
	}

private:
	Common::SharedPtr<Bitmap> _bitmap;
	Point _offset;
};

}

#endif // TOPGUN_CELL_H

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

#ifndef TOPGUN_BITMAP_H
#define TOPGUN_BITMAP_H

#include "topgun/Resource.h"

namespace TopGun {

class Bitmap : public ISurfaceResource {
public:
	static constexpr ResourceType kResourceType = ResourceType::kBitmap;

	Bitmap(uint32 index);
	virtual ~Bitmap() = default;

	virtual bool load(Common::Array<byte> &&data) override;

	virtual Point getOffset() const override {
		return _offset;
	}

	virtual Graphics::Surface *getSurface() override {
		return _surface->surfacePtr();
	}

private:
	void decompressSimpleRLE(Common::SeekableReadStream &stream, uint32 width, uint32 height);
	void decompressComplexRLE(Common::SeekableReadStream &stream, uint32 width, uint32 height);

private:
	ScopedPtr<Graphics::ManagedSurface> _surface;
	Point _offset;
};

}

#endif // TOPGUN_BITMAP_H

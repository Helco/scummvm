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

#include "topgun/topgun.h"
#include "topgun/graphics/Cell.h"

namespace TopGun {

Cell::Cell(uint32 index) : ISurfaceResource(kResourceType, index) {
}

bool Cell::load(Common::Array<byte> &&data) {
	assert(g_engine->getResourceFile()->_architecture == Architecture::kBits32);
	auto stream = Common::MemorySeekableReadWriteStream(data.data(), data.size());

	_bitmap = g_engine->loadResource<Bitmap>(stream.readUint32LE());
	stream.skip(4);
	_offset.x = stream.readSint32LE();
	_offset.y = stream.readSint32LE();

	return !stream.err();
}

}

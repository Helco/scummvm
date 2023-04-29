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
#include "Bitmap.h"

namespace TopGun {

Bitmap::Bitmap(uint32 index) :
	ISurfaceResource(kResourceType, index),
	_surface(new Graphics::ManagedSurface()) {
}

bool Bitmap::load(Common::Array<byte> &&data) {
	auto architecture = g_engine->getResourceFile()->_architecture;
	assert(architecture == Architecture::kBits32);
	auto stream = Common::MemorySeekableReadWriteStream(data.data(), data.size());

	const auto width = stream.readUint32LE();
	const auto height = stream.readUint32LE();
	const auto flags = stream.readUint32LE();
	_offset.x = stream.readSint32LE();
	_offset.y = stream.readSint32LE();
	stream.skip(4);

	if (flags & 0x80000000)
		decompressComplexRLE(stream, width, height);
	else
		decompressSimpleRLE(stream, width, height);

	return !stream.err();
}

void Bitmap::decompressSimpleRLE(Common::SeekableReadStream &stream, uint32 width, uint32 height) {
	const auto alignedWidth = (width + 3) & ~3u;
	const auto unaligned = alignedWidth - width;
	auto pixels = (byte *)malloc(alignedWidth * height);
	if (pixels == nullptr)
		error("Could not allocate %d bytes for bitmap", alignedWidth * height);
	getSurface()->init(width, height, alignedWidth, pixels, Graphics::PixelFormat::createFormatCLUT8());

	stream.skip(2 * height); // for some reason there are just some unused bytes

	byte *destPtr = (byte *)pixels;
	byte packetType;
	while (true) {
		packetType = stream.readByte();
		if (!packetType) {
			destPtr += unaligned;
			packetType = stream.readByte();
		}
		if (!packetType)
			break;

		assert(destPtr - pixels <= alignedWidth * height);
		if (packetType < 128) { // repeat packet
			const auto packetSize = packetType;
			memset(destPtr, stream.readByte(), packetSize);
			destPtr += packetSize;
		} else { // copy packet
			byte packetSize = ~packetType;
			if (!packetSize)
				packetSize = stream.readByte();
			stream.read(destPtr, packetSize);
			destPtr += packetSize;
		}
	}
}

void Bitmap::decompressComplexRLE(Common::SeekableReadStream &stream, uint32 width, uint32 height) {
	error("Complex RLE is not supported yet");
}

}

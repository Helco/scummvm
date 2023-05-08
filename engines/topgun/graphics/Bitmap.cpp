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

#include "common/bitstream.h"
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

	if (flags & 0x00000080)
		decompressSimpleRLE(stream, width, height);
	else
		decompressComplexRLE(stream, width, height);
	if (!(flags & 0x40))
		getSurface()->flipVertical(Rect(width, height));

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
	const auto alignedWidth = (width + 3) & ~3u;
	auto pixels = (byte *)malloc(alignedWidth * height);
	if (pixels == nullptr)
		error("Could not allocate %d bytes for bitmap", alignedWidth * height);
	getSurface()->init(width, height, alignedWidth, pixels, Graphics::PixelFormat::createFormatCLUT8());

	Array<Symbol> symbols;
	byte *destPtr = (byte *)pixels;
	while (true) {
		const uint16 packetHeader = stream.readUint16LE();
		const uint16 packetType = packetHeader & 0xE000;
		const uint16 packetSize = packetHeader & 0x1FFF;

		int bits;
		switch (packetType)
		{
		case 0xE000: return;
		case 0:
			stream.read(destPtr, packetSize);
			destPtr += packetSize;
			continue;
		case 0x4000: bits = 10; break;
		case 0x6000: bits = 11; break;
		case 0x8000: bits = 12; break;
		default:
			error("Invalid data in complex RLE bitmap");
			return;
		}

		symbols.reserve((1 << bits) - 1);
		const auto packetEnd = stream.pos() + packetSize;
		while (stream.pos() < packetEnd)
		{
			const auto subPacketSize = stream.readUint16LE();
			const auto subPacketEnd = stream.pos() + subPacketSize;
			decompressLZWPacket(stream, destPtr, bits, symbols);
			stream.seek(subPacketEnd, SEEK_SET);
		}
	}
	if (stream.err())
		error("Stream error in complex RLE bitmap");
}

void Bitmap::decompressLZWPacket(Common::SeekableReadStream &stream, byte *&destPtr, int bits, Array<Symbol> &symbols) {
	const auto maxSymbols = (1 << bits) - 1;
	symbols.resize(256);
	Common::BitStream8MSB bitStream(stream);

	uint16 symbol = bitStream.getBits(bits);
	*destPtr++ = (byte)symbol;
	uint16 prevSymbol = symbol;
	byte newLastData, lastData = (byte)symbol;

	while (true) {
		symbol = bitStream.getBits(bits);
		if (symbol == maxSymbols)
			break;
		else if (symbol < symbols.size())
			pushSymbol(destPtr, symbol, symbols, newLastData);
		else {
			pushSymbol(destPtr, prevSymbol, symbols, newLastData);
			*destPtr++ = lastData;
		}
		lastData = newLastData;

		if (symbols.size() < maxSymbols)
			symbols.push_back(Symbol{ prevSymbol, lastData, (byte)(symbols[prevSymbol].length + 1) });
		prevSymbol = symbol;
	}
}

void Bitmap::pushSymbol(byte *&destPtr, uint16 symbol, const Array<Symbol> &symbols, byte &newLastData) {
	auto length = symbols[symbol].length;
	for (int i = 0; i < length; i++)
	{
		destPtr[length - i] = symbols[symbol].data;
		symbol = symbols[symbol].prevSymbol;
	}
	newLastData = destPtr[0] = (byte)symbol;
	destPtr += length + 1;
}

}

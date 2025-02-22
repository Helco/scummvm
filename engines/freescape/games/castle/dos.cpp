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

#include "common/file.h"
#include "common/memstream.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

Common::SeekableReadStream *CastleEngine::decryptFile(const Common::Path &filename) {
	Common::File file;
	file.open(filename);
	if (!file.isOpen())
		error("Failed to open %s", filename.toString().c_str());

	int size = file.size();
	byte *encryptedBuffer = (byte *)malloc(size);
	file.read(encryptedBuffer, size);
	file.close();

	int seed = 24;
	for (int i = 0; i < size; i++) {
		encryptedBuffer[i] ^= seed;
		seed = (seed + 1) & 0xff;
	}

	return (new Common::MemoryReadStream(encryptedBuffer, size));
}

extern byte kEGADefaultPalette[16][3];
extern Common::MemoryReadStream *unpackEXE(Common::File &ms);

void CastleEngine::loadAssetsDOSFullGame() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		_viewArea = Common::Rect(40, 33, 280, 152);

		file.open("CME.EXE");
		stream = unpackEXE(file);
		if (stream) {
			loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200);
		}

		delete stream;
		file.close();

		file.open("CMLE.DAT");
		_title = load8bitBinImage(&file, 0x0);
		_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CMOE.DAT");
		_option = load8bitBinImage(&file, 0x0);
		_option->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CME.DAT");
		_border = load8bitBinImage(&file, 0x0);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		switch (_language) {
			case Common::ES_ESP:
				stream = decryptFile("CMLS");
				break;
			case Common::FR_FRA:
				stream = decryptFile("CMLF");
				break;
			case Common::DE_DEU:
				stream = decryptFile("CMLG");
				break;
			case Common::EN_ANY:
				stream = decryptFile("CMLE");
				break;
			default:
				error("Invalid or unsupported language: %x", _language);
		}

		loadFonts(kFreescapeCastleFont, 59);
		loadMessagesVariableSize(stream, 0x11, 164);
		delete stream;

		stream = decryptFile("CMEDF");
		load8bitBinary(stream, 0, 16);
		for (auto &it : _areaMap)
			it._value->addStructure(_areaMap[255]);

		_areaMap[1]->addFloor();
		_areaMap[2]->addFloor();
		delete stream;
	} else
		error("Not implemented yet");

	addGhosts();
	// CPC
	// file = gameDir.createReadStreamForMember("cm.bin");
	// if (file == nullptr)
	//	error("Failed to open cm.bin");
	// load8bitBinary(file, 0x791a, 16);
}

void CastleEngine::loadAssetsDOSDemo() {
	Common::File file;
	Common::SeekableReadStream *stream = nullptr;

	if (_renderMode == Common::kRenderEGA) {
		_viewArea = Common::Rect(40, 33, 280, 152);

		file.open("CMDE.EXE");
		stream = unpackEXE(file);
		if (stream) {
			loadSpeakerFxDOS(stream, 0x636d + 0x200, 0x63ed + 0x200);
		}

		delete stream;
		file.close();

		file.open("CMLE.DAT");
		_title = load8bitBinImage(&file, 0x0);
		_title->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CMOE.DAT");
		_option = load8bitBinImage(&file, 0x0);
		_option->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		file.open("CME.DAT");
		_border = load8bitBinImage(&file, 0x0);
		_border->setPalette((byte *)&kEGADefaultPalette, 0, 16);
		file.close();

		stream = decryptFile("CMLD"); // Only english
		loadFonts(kFreescapeCastleFont, 59);
		loadMessagesVariableSize(stream, 0x11, 164);
		loadRiddles(stream, 0xaae, 10);

		/*for (int i = 0; i < 16; i++) {
			debug("%lx", stream->pos());
			for (int j = 0; j < 16; j++) {
				byte c = stream->readByte();
				debugN("%x/%c", c, c);
			}
			debugN("\n");
		}*/
		delete stream;

		stream = decryptFile("CDEDF");
		load8bitBinary(stream, 0, 16);
		for (auto &it : _areaMap)
			it._value->addStructure(_areaMap[255]);

		_areaMap[1]->addFloor();
		_areaMap[2]->addFloor();
		delete stream;
	} else
		error("Not implemented yet");
}

void CastleEngine::drawDOSUI(Graphics::Surface *surface) {
	uint32 color = 10;
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	color = 0;

	_gfx->readFromPalette(color, r, g, b);
	uint32 back = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::Rect backRect(97, 181, 232, 190);
	surface->fillRect(backRect, back);

	Common::String message;
	int deadline;
	getLatestMessages(message, deadline);
	if (deadline <= _countdown) {
		drawStringInSurface(message, 97, 182, front, back, surface);
		_temporaryMessages.push_back(message);
		_temporaryMessageDeadlines.push_back(deadline);
	} else
		drawStringInSurface(_currentArea->_name, 97, 182, front, back, surface);
}

} // End of namespace Freescape

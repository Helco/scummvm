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
#include "common/debug.h"
#include "topgun/topgun.h"

constexpr uint16 kMagic = 0x4C37;

namespace TopGun {

ResourceFile::~ResourceFile() {
	for (auto pair : _extensionFiles)
		delete pair._value;
}

bool ResourceFile::load(const Common::String &filename) {
	_baseExtensionPath = filename.substr(0, filename.size() - 3);

	if (!_mainFile.open(filename) ||
		_mainFile.readUint16LE() != kMagic)
		return false;

	const uint16 headerSize = _mainFile.readUint16LE(); // the combined size of these first fields and the version-dependent header
	_architecture = (Architecture)_mainFile.readUint16LE();
	if (!readTitles(_mainFile))
		return false;
	_version = (ResourceFileVersion)_mainFile.readUint16LE();

	switch (_architecture)
	{
	case Architecture::kBits32:
		if (!readHeaderFor32Bit(_mainFile, headerSize))
			return false;
		break;
	case Architecture::kBits16:
		if (!readHeaderFor16Bit(_mainFile, headerSize))
			return false;
		break;
	case Architecture::kGrail2:
		if (!readHeaderForGrail2(_mainFile, headerSize))
			return false;
	default:
		return false;
	}

	if (!readResourceLocations(_mainFile) ||
		!readVariables(_mainFile) ||
		!readStringKeyResource(_mainFile, KeyResource::kConstStrings, _constStrings) ||
		!readPalette(_mainFile) ||
		!readStringKeyResource(_mainFile, KeyResource::kPlugins, _plugins) ||
		!readStringKeyResource(_mainFile, KeyResource::kPluginProcs, _pluginProcedures) ||
		!readPluginIndices(_mainFile) ||
		_pluginProcedures.size() != _pluginIndexPerProcedure.size())
		return false;

	return !_mainFile.err();
}

bool ResourceFile::readTitles(Common::SeekableReadStream &stream) {
	auto bytesLeft = 79;
	stream.skip(1); // the size of both titles, we do not need it
	_title = stream.readString(0);
	bytesLeft -= _title.size() + 1;
	_subTitle = stream.readString(0);
	bytesLeft -= _subTitle.size() + 1;
	stream.skip(bytesLeft);
	return !stream.err();
}

bool ResourceFile::readHeaderFor32Bit(Common::SeekableReadStream &stream, uint16 headerSize) {
	assert(headerSize == 476);

	_entryId = stream.readUint32LE();

	constexpr size_t kMaxScriptEndOffsets = 0x30;
	_scriptEndOffsets.reserve(kMaxScriptEndOffsets);
	for (size_t i = 0; i < kMaxScriptEndOffsets; i++)
		_scriptEndOffsets.push_back(stream.readUint32LE());

	const uint32 scriptCount = stream.readUint32LE();
	if (scriptCount > kMaxScriptEndOffsets)
		return false;
	_scriptEndOffsets.resize(scriptCount);

	_maxFadeColors = stream.readUint32LE();
	_maxTransColors = stream.readUint32LE();
	_dynamicResources = stream.readUint32LE();
	_dynamicStringCount = stream.readUint32LE();
	stream.skip(4); // titled as variable count but we do not have to trust this value
	_maxScrMsg = stream.readUint32LE();
	stream.skip(44);

	constexpr size_t kKeyResourceCount = 15;
	for (size_t i = 0; i < kKeyResourceCount; i++)
	{
		_keyResources[i]._offset = stream.readUint32LE();
		_keyResources[i]._size = stream.readUint32LE();
	}

	return !stream.err();
}

bool ResourceFile::readHeaderFor16Bit(Common::SeekableReadStream &stream, uint16 headerSize) {
	assert(headerSize == 354);

	_entryId = 0;
	stream.skip(10);

	constexpr size_t kMaxScriptEndOffsets = 0x20;
	_scriptEndOffsets.reserve(kMaxScriptEndOffsets);
	for (size_t i = 0; i < kMaxScriptEndOffsets; i++)
		_scriptEndOffsets.push_back(stream.readUint32LE());

	const uint16 scriptCount = stream.readUint16LE();
	if (scriptCount > kMaxScriptEndOffsets)
		return false;
	_scriptEndOffsets.resize(scriptCount);

	_maxFadeColors = stream.readUint16LE();
	_maxTransColors = stream.readUint16LE();
	_dynamicResources = stream.readUint16LE();
	_dynamicStringCount = stream.readUint16LE();
	stream.skip(6);
	_maxScrMsg = stream.readUint16LE();
	stream.skip(4);

	constexpr size_t kKeyResourceCount = 14;
	for (size_t i = 0; i < kKeyResourceCount; i++) {
		_keyResources[i]._offset = stream.readUint32LE();
		_keyResources[i]._size = stream.readUint32LE();
	}

	return !stream.err();
}

bool ResourceFile::readHeaderForGrail2(Common::SeekableReadStream &stream, uint16 headerSize) {
	assert(headerSize == 322);

	_entryId = 0;
	stream.skip(10);

	constexpr size_t kMaxScriptEndOffsets = 0x18;
	_scriptEndOffsets.reserve(kMaxScriptEndOffsets);
	for (size_t i = 0; i < kMaxScriptEndOffsets; i++)
		_scriptEndOffsets.push_back(stream.readUint32LE());

	const uint16 scriptCount = stream.readUint16LE();
	if (scriptCount > kMaxScriptEndOffsets)
		return false;
	_scriptEndOffsets.resize(scriptCount);

	_maxFadeColors = stream.readUint16LE();
	_maxTransColors = stream.readUint16LE();
	_dynamicResources = stream.readUint16LE();
	_dynamicStringCount = stream.readUint16LE();
	stream.skip(22);
	_maxScrMsg = UINT32_MAX;

	constexpr size_t kKeyResourceCount = 14;
	for (size_t i = 0; i < kKeyResourceCount; i++) {
		_keyResources[i]._offset = stream.readUint32LE();
		_keyResources[i]._size = stream.readUint32LE();
	}

	return !stream.err();
}

bool ResourceFile::readResourceLocations(Common::SeekableReadStream &stream) {
	const auto range = _keyResources[(int)KeyResource::kResources];
	_staticResources = range._size / 10;
	_totalResources = _staticResources + _dynamicResources;
	_resources.resize(_totalResources);

	if (!stream.seek(range._offset, SEEK_SET))
		return false;
	for (size_t i = 0; i < _staticResources; i++) {
		_resources[i]._type = (ResourceType)stream.readByte();
		_resources[i]._extension = stream.readByte();
		_resources[i]._offset = stream.readUint32LE();
		_resources[i]._size = stream.readUint32LE();
	}

	return !stream.err();
}

bool ResourceFile::readVariables(Common::SeekableReadStream &stream) {
	const auto range = _keyResources[(int)KeyResource::kVariables];

	if (!stream.seek(range._offset, SEEK_SET))
		return false;

	if (_architecture == Architecture::kBits32) {
		_variables.resize(range._size / 8);
		for (size_t i = 0; i < _variables.size(); i++) {
			_variables[i]._key = stream.readUint32LE();
			_variables[i]._value = stream.readSint32LE();
		}
	} else {
		_variables.resize(range._size / 4);
		for (size_t i = 0; i < _variables.size(); i++) {
			_variables[i]._key = stream.readUint16LE();
			_variables[i]._value = stream.readSint16LE();
		}
	}

	return !stream.err();
}

bool ResourceFile::readStringKeyResource(Common::SeekableReadStream &stream, KeyResource keyResource, Common::Array<Common::String> &array) {
	const auto range = _keyResources[(int)keyResource];
	const int64 endOffset = range._offset + range._size;
	if (!stream.seek(range._offset, SEEK_SET))
		return false;

	while (stream.pos() < endOffset)
		array.push_back(stream.readString());

	return !stream.err() && stream.pos() == endOffset;
}

bool ResourceFile::readPalette(Common::SeekableReadStream &stream) {
	const auto range = _keyResources[(int)KeyResource::kPalette];
	_palette.resize(3 * range._size / 4);
	if (!stream.seek(range._offset, SEEK_SET))
		return false;

	for (size_t i = 0; i < _palette.size() / 3; i += 3) {
		_palette[i + 0] = stream.readByte();
		_palette[i + 1] = stream.readByte();
		_palette[i + 2] = stream.readByte();
		stream.skip(1);
	}

	return !stream.err();
}

bool ResourceFile::readPluginIndices(Common::SeekableReadStream& stream) {
	const auto range = _keyResources[(int)KeyResource::kPluginIndexPerProc];
	if (!stream.seek(range._offset, SEEK_SET))
		return false;

	if (_architecture == Architecture::kBits32) {
		_pluginIndexPerProcedure.resize(range._size / 4);
		for (size_t i = 0; i < _pluginIndexPerProcedure.size(); i++)
			_pluginIndexPerProcedure[i] = stream.readUint32LE();
	} else {
		_pluginIndexPerProcedure.resize(range._size / 2);
		for (size_t i = 0; i < _pluginIndexPerProcedure.size(); i++)
			_pluginIndexPerProcedure[i] = stream.readUint16LE();
	}

	return !stream.err();
}

Common::Array<byte> ResourceFile::loadResource(uint32 index) {
	const auto location = _resources[index];
	Common::File *file = &_mainFile;
	size_t additionalOffset = 0;

	if (location._type >= ResourceType::kMovie && location._type <= ResourceType::kTile) {
		additionalOffset = _keyResources[(int)KeyResource::kScripts]._offset;
	}
	else if (_version == ResourceFileVersion::kUseExtensionFiles) {
		file = _extensionFiles[location._extension];
		if (file == nullptr) {
			debugCN(kInfo, kDebugResource, "Loading extension file %d", location._extension);
			auto extensionPath = Common::String::format("%s%03d", _baseExtensionPath.c_str(), location._extension);
			file = new Common::File();
			if (!file->open(extensionPath))
				error("Could not open resource extension file: %s", extensionPath.c_str());
			_extensionFiles[location._extension] = file;
		}
	}

	Common::Array<byte> result(location._size);
	if (!file->seek(additionalOffset + location._offset, SEEK_SET) ||
		file->read(result.begin(), result.size()) != result.size())
		error("Could not read resource %d", index);
	return result;
}

}

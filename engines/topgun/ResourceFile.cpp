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
	if (!readTitles())
		return false;
	_version = (ResourceFileVersion)_mainFile.readUint16LE();

	switch (_architecture)
	{
	case Architecture::kBits32:
		if (!readHeaderFor32Bit(headerSize))
			return false;
		break;
	case Architecture::kBits16:
		if (!readHeaderFor16Bit(headerSize))
			return false;
		break;
	case Architecture::kGrail2:
		if (!readHeaderForGrail2(headerSize))
			return false;
	default:
		return false;
	}

	if (!readResourceLocations() ||
		!readVariables() ||
		!readConstStringData() ||
		!readPalette() ||
		!readStringKeyResource(KeyResource::kPlugins, _plugins) ||
		!readStringKeyResource(KeyResource::kPluginProcs, _pluginProcedures) ||
		!readPluginIndices() ||
		_pluginProcedures.size() != _pluginIndexPerProcedure.size())
		return false;

	return !_mainFile.err();
}

bool ResourceFile::readTitles() {
	auto bytesLeft = 79;
	_mainFile.skip(1); // the size of both titles, we do not need it
	_title = _mainFile.readString(0);
	bytesLeft -= _title.size() + 1;
	_subTitle = _mainFile.readString(0);
	bytesLeft -= _subTitle.size() + 1;
	_mainFile.skip(bytesLeft);
	return !_mainFile.err();
}

bool ResourceFile::readHeaderFor32Bit( uint16 headerSize) {
	assert(headerSize == 476);

	_entryId = _mainFile.readUint32LE();

	constexpr size_t kMaxScriptEndOffsets = 0x30;
	_scriptEndOffsets.reserve(kMaxScriptEndOffsets);
	for (size_t i = 0; i < kMaxScriptEndOffsets; i++)
		_scriptEndOffsets.push_back(_mainFile.readUint32LE());

	const uint32 scriptCount = _mainFile.readUint32LE();
	if (scriptCount > kMaxScriptEndOffsets)
		return false;
	_scriptEndOffsets.resize(scriptCount);

	_maxFadeColors = _mainFile.readUint32LE();
	_maxTransColors = _mainFile.readUint32LE();
	_dynamicResources = _mainFile.readUint32LE();
	_dynamicStringCount = _mainFile.readUint32LE();
	_mainFile.skip(4); // titled as variable count but we do not have to trust this value
	_maxScrMsg = _mainFile.readUint32LE();
	_mainFile.skip(44);

	constexpr size_t kKeyResourceCount = 15;
	for (size_t i = 0; i < kKeyResourceCount; i++)
	{
		_keyResources[i]._offset = _mainFile.readUint32LE();
		_keyResources[i]._size = _mainFile.readUint32LE();
	}

	return !_mainFile.err();
}

bool ResourceFile::readHeaderFor16Bit( uint16 headerSize) {
	assert(headerSize == 354);

	_entryId = 0;
	_mainFile.skip(10);

	constexpr size_t kMaxScriptEndOffsets = 0x20;
	_scriptEndOffsets.reserve(kMaxScriptEndOffsets);
	for (size_t i = 0; i < kMaxScriptEndOffsets; i++)
		_scriptEndOffsets.push_back(_mainFile.readUint32LE());

	const uint16 scriptCount = _mainFile.readUint16LE();
	if (scriptCount > kMaxScriptEndOffsets)
		return false;
	_scriptEndOffsets.resize(scriptCount);

	_maxFadeColors = _mainFile.readUint16LE();
	_maxTransColors = _mainFile.readUint16LE();
	_dynamicResources = _mainFile.readUint16LE();
	_dynamicStringCount = _mainFile.readUint16LE();
	_mainFile.skip(6);
	_maxScrMsg = _mainFile.readUint16LE();
	_mainFile.skip(4);

	constexpr size_t kKeyResourceCount = 14;
	for (size_t i = 0; i < kKeyResourceCount; i++) {
		_keyResources[i]._offset = _mainFile.readUint32LE();
		_keyResources[i]._size = _mainFile.readUint32LE();
	}

	return !_mainFile.err();
}

bool ResourceFile::readHeaderForGrail2( uint16 headerSize) {
	assert(headerSize == 322);

	_entryId = 0;
	_mainFile.skip(10);

	constexpr size_t kMaxScriptEndOffsets = 0x18;
	_scriptEndOffsets.reserve(kMaxScriptEndOffsets);
	for (size_t i = 0; i < kMaxScriptEndOffsets; i++)
		_scriptEndOffsets.push_back(_mainFile.readUint32LE());

	const uint16 scriptCount = _mainFile.readUint16LE();
	if (scriptCount > kMaxScriptEndOffsets)
		return false;
	_scriptEndOffsets.resize(scriptCount);

	_maxFadeColors = _mainFile.readUint16LE();
	_maxTransColors = _mainFile.readUint16LE();
	_dynamicResources = _mainFile.readUint16LE();
	_dynamicStringCount = _mainFile.readUint16LE();
	_mainFile.skip(22);
	_maxScrMsg = UINT32_MAX;

	constexpr size_t kKeyResourceCount = 14;
	for (size_t i = 0; i < kKeyResourceCount; i++) {
		_keyResources[i]._offset = _mainFile.readUint32LE();
		_keyResources[i]._size = _mainFile.readUint32LE();
	}

	return !_mainFile.err();
}

bool ResourceFile::readResourceLocations() {
	const auto range = _keyResources[(int)KeyResource::kResources];
	_staticResources = range._size / 10;
	_totalResources = _staticResources + _dynamicResources;
	_resources.resize(_totalResources);

	if (!_mainFile.seek(range._offset, SEEK_SET))
		return false;
	for (size_t i = 0; i < _staticResources; i++) {
		_resources[i]._type = (ResourceType)_mainFile.readByte();
		_resources[i]._extension = _mainFile.readByte();
		_resources[i]._offset = _mainFile.readUint32LE();
		_resources[i]._size = _mainFile.readUint32LE();
	}

	return !_mainFile.err();
}

bool ResourceFile::readVariables() {
	const auto range = _keyResources[(int)KeyResource::kVariables];

	if (!_mainFile.seek(range._offset, SEEK_SET))
		return false;

	if (_architecture == Architecture::kBits32) {
		_variables.resize(range._size / 8);
		for (size_t i = 0; i < _variables.size(); i++) {
			_variables[i]._key = _mainFile.readUint32LE();
			_variables[i]._value = _mainFile.readSint32LE();
		}
	} else {
		_variables.resize(range._size / 4);
		for (size_t i = 0; i < _variables.size(); i++) {
			_variables[i]._key = _mainFile.readUint16LE();
			_variables[i]._value = _mainFile.readSint16LE();
		}
	}

	return !_mainFile.err();
}

bool ResourceFile::readConstStringData() {
	const auto range = _keyResources[(int)KeyResource::kConstStrings];
	if (!_mainFile.seek(range._offset, SEEK_SET))
		return false;

	_constStringData.resize(range._size);
	_mainFile.read(_constStringData.data(), range._size);
	return !_mainFile.err();
}

bool ResourceFile::readStringKeyResource( KeyResource keyResource, Common::Array<Common::String> &array) {
	const auto range = _keyResources[(int)keyResource];
	const int64 endOffset = range._offset + range._size;
	if (!_mainFile.seek(range._offset, SEEK_SET))
		return false;

	while (_mainFile.pos() < endOffset)
		array.push_back(_mainFile.readString());

	return !_mainFile.err() && _mainFile.pos() == endOffset;
}

bool ResourceFile::readPalette() {
	const auto range = _keyResources[(int)KeyResource::kPalette];
	_palette.resize(3 * range._size / 4);
	if (!_mainFile.seek(range._offset, SEEK_SET))
		return false;

	for (size_t i = 0; i < _palette.size(); i += 3) {
		_palette[i + 0] = _mainFile.readByte();
		_palette[i + 1] = _mainFile.readByte();
		_palette[i + 2] = _mainFile.readByte();
		_mainFile.skip(1);
	}

	return !_mainFile.err();
}

bool ResourceFile::readPluginIndices() {
	const auto range = _keyResources[(int)KeyResource::kPluginIndexPerProc];
	if (!_mainFile.seek(range._offset, SEEK_SET))
		return false;

	if (_architecture == Architecture::kBits32) {
		_pluginIndexPerProcedure.resize(range._size / 4);
		for (size_t i = 0; i < _pluginIndexPerProcedure.size(); i++)
			_pluginIndexPerProcedure[i] = _mainFile.readUint32LE();
	} else {
		_pluginIndexPerProcedure.resize(range._size / 2);
		for (size_t i = 0; i < _pluginIndexPerProcedure.size(); i++)
			_pluginIndexPerProcedure[i] = _mainFile.readUint16LE();
	}

	return !_mainFile.err();
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
			debugCN(kInfo, kDebugResource, "Loading extension file %d\n", location._extension);
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

const char *ResourceFile::getConstString(uint32 index) const {
	return &_constStringData[index];
}

}

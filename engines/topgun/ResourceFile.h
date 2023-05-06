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

#ifndef RESOURCEFILE_H
#define RESOURCEFILE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/array.h"
#include "common/hashmap.h"
#include "common/file.h"

namespace TopGun {
enum class Architecture : uint16 {
	kBits16 = 0x3631,
	kBits32 = 0x3233,
	kGrail2 = 2 ///< also a 16-bit architecture
};

enum class ResourceFileVersion : uint16 {
	kOnlyMainFile = 2,
	kUseExtensionFiles = 258
};

enum class KeyResource {
	kResources = 0,
	kEntries,
	kIndexBuffers,
	kVariables,
	kConstStrings,
	kScripts,
	kPalette,
	kNameTable,
	kUnknown8,
	kUnknown9,
	kPlugins,
	kPluginProcs,
	kPluginIndexPerProc,
	kUnknown13,
	kSourceFile,

	kCount
};

enum class ResourceType {
	kInvalid = 0,
	kBitmap,
	kData,
	kFile,
	kFrame,
	kGround,
	kMidi,
	kModel,
	kMProto,
	kObj3D,
	kOProto,
	kTable,
	kWave,
	kMovie,
	kArray,
	kCell,
	kGroup,
	kPalette,
	kQueue,
	kScript,
	kSprite,
	kText,
	kTile,
	kTitle,
	kSubtitle,
	kLocal,
	kEntry
};

struct ResourceLocation {
	ResourceType _type = ResourceType::kInvalid;
	uint8 _extension = 0;
	uint32 _offset = 0, _size = 0;
};

struct KeyResourceLocation {
	uint32 _offset, _size;

	inline bool isPresent() {
		return _size > 0;
	}
};

struct VariableEntry {
	uint32 _key;
	int32 _value;
};

class ResourceFile {
public:
	~ResourceFile();
	bool load(const Common::String &filename);

	const char *getConstString(uint32 offset) const;
	Common::Array<byte> loadResource(uint32 index);

private:
	bool readTitles();
	bool readHeaderFor32Bit(uint16 headerSize);
	bool readHeaderFor16Bit(uint16 headerSize);
	bool readHeaderForGrail2(uint16 headerSize);
	bool readResourceLocations();
	bool readVariables();
	bool readConstStringData();
	bool readStringKeyResource( KeyResource keyResource, Common::Array<Common::String> &array);
	bool readPalette();
	bool readPluginIndices();

public:
	Architecture _architecture = Architecture::kBits32;
	ResourceFileVersion _version = ResourceFileVersion::kUseExtensionFiles;
	Common::String _title, _subTitle;
	uint32 _entryId = 0,
		_staticResources = 0,
		_dynamicResources = 0,
		_totalResources = 0,
		_dynamicStringCount = 0,
		_maxFadeColors = 0,
		_maxTransColors = 0,
		_maxScrMsg = 0;

	KeyResourceLocation _keyResources[(size_t)KeyResource::kCount];
	Common::Array<ResourceLocation> _resources;
	Common::Array<VariableEntry> _variables;
	Common::Array<char> _constStringData;
	Common::Array<uint32> _scriptEndOffsets;
	Common::Array<byte> _palette; ///< The main palette, which can be changed e.g. using palette resources
	Common::Array<Common::String> _plugins;
	Common::Array<Common::String> _pluginProcedures;
	Common::Array<uint32> _pluginIndexPerProcedure;

	Common::File _mainFile;
	Common::HashMap<byte, Common::File*> _extensionFiles;
	Common::String _baseExtensionPath;
};

}

#endif // RESOURCEFILE_H

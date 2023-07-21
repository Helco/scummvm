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

#ifndef TOPGUN_DETECTION_H
#define TOPGUN_DETECTION_H

#include "engines/advancedDetector.h"

namespace TopGun {

enum TopgunDebugChannels {
	kDebugRuntime  = 1 << 0,
	kDebugScript   = 1 << 1,
	kDebugSprite   = 1 << 2,
	kDebugResource = 1 << 3,
	kDebugAudio    = 1 << 4,
};

enum TopgunDebugLevel {
	kInfo = 0,
	kTrace,
	kVerbose,
	kSuperVerbose
};

extern const PlainGameDescriptor topgunGames[];

struct TopGunGameDescription {
	ADGameDescription _baseDescription;
	size_t _sceneVarCount;
	size_t _systemVarCount;

	inline size_t globalVarCount() const {
		return _sceneVarCount + _systemVarCount;
	}
};

extern const TopGunGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Topgun

class TopGunMetaEngineDetection : public AdvancedMetaEngineDetection {
	static const DebugChannelDef debugFlagList[];

public:
	TopGunMetaEngineDetection();
	~TopGunMetaEngineDetection() override {}

	const char *getName() const override {
		return "topgun";
	}

	const char *getEngineName() const override {
		return "TopGun";
	}

	const char *getOriginalCopyright() const override {
		return "TopGun (C) Copyright 1996 7th Level, Inc.";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // TOPGUN_DETECTION_H

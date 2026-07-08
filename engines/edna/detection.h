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

#ifndef EDNA_DETECTION_H
#define EDNA_DETECTION_H

#include "engines/advancedDetector.h"

namespace Edna {

enum EdnaDebugChannels {
	kDebugGameplay = 1,
	kDebugGraphics,
	kDebugScript,
};

extern const PlainGameDescriptor ednaGames[];

extern const ADGameDescription gameDescriptions[];

#define GAMEOPTION_ORIGINAL_SAVELOAD GUIO_GAMEOPTIONS1

} // End of namespace Edna

class EdnaMetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	EdnaMetaEngineDetection();
	~EdnaMetaEngineDetection() override {}

	const char *getName() const override {
		return "edna";
	}

	const char *getEngineName() const override {
		return "Edna";
	}

	const char *getOriginalCopyright() const override {
		return "Edna (C)";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif // EDNA_DETECTION_H

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

#ifndef EDNA_TRANSLATION_H
#define EDNA_TRANSLATION_H

#include "edna/util.h"

#include "common/language.h"

namespace Edna {

// The game has some hardcoded translated strings that we keep here
// eventually we will want to support arbitrary translation lookups
// e.g. to support the Anniversary Edition data or fan-made translations

struct MiscTranslations;

class Translation {
public:
	Translation(Common::Language language);

	const char *action(PlayerAction action) const;
	const char *actionWith() const; // for "Use <item> *with* <target>"
	const char *dropTopic() const;

private:
	const Common::Language _language;
	const char *const *_actionNames = nullptr;
	const MiscTranslations *_misc = nullptr;
};

}

#endif // EDNA_TRANSLATION_H

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

#include "edna/translation.h"

using namespace Common;

namespace Edna {

static constexpr const char *const kActionNamesDE[] = {
	"",
	"Schaue an",
	"Benutze",
	"Nimm",
	"Rede mit",
	"Gehe zu",
	"zu Harvey",
	"zu Edna",
	"Was ist",
	"Rede mit Edna über",
};
static constexpr const char *const kActionNamesEN[] = {
	"",
	"Look at",
	"Use",
	"Pick up",
	"Talk to",
	"Walk to"
	"to Harvey",
	"to Edna",
	"What is",
	"Talk to Edna about"
};

struct MiscTranslations {
	const char *_actionWith;
};
static constexpr const MiscTranslations kMiscDE = {
	"mit"
};
static constexpr const MiscTranslations kMiscEN = {
	"with"
};

Translation::Translation(Language language) : _language(language) {
	switch (language) {
	default:
		warning("Unimplemented edna language: %s", Common::getLanguageDescription(language));
		// fall through
	case Language::EN_ANY:
		_actionNames = kActionNamesEN;
		_misc = &kMiscEN;
		break;
	case Language::DE_DEU:
		_actionNames = kActionNamesDE;
		_misc = &kMiscDE;
		break;
	}
}

const char *Translation::action(PlayerAction action) const {
	assert(action >= PlayerAction::None && action <= PlayerAction::TalkAbout);
	return _actionNames[(uint)action];
}

const char *Translation::actionWith() const {
	return _misc->_actionWith;
}

}

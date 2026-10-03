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

#include "common/translation.h"

using namespace Common;

namespace Edna {


static constexpr const char *const kActionNames[] = {
	"",
	_s("Look at"),
	_s("Use"),
	_s("Pick up"),
	_s("Talk to"),
	_s("Walk to"),
	_s("to Harvey"),
	_s("to Edna"),
	_s("What is"),
	_s("Talk to Edna about")
};

static constexpr const char *const kInfoJokes[] = {
	_s("Ahhh!"),
	_s("Hey!"),
	_s("Stop it!"),
	_s("Well, click somewhere else!")
};

Translation::Translation()
	: _dropTopic(_("Discord")) // used as display name for the topic row
	, _toggleMusic(_("Music on / off")) // used in the options menu
	, _toggleSound(_("Sound on / off"))
	, _toggleSubtitles(_("Text on / off"))
	, _textSpeed(_("Text velocity")) {
	for (uint i = 0; i < kPlayerActionCount; i++)
		_actionNames[i] = _(kActionNames[i]).encode();
}

const char *Translation::action(PlayerAction action) const {
	assert((uint)action < kPlayerActionCount);
	return _actionNames[(uint)action].c_str();
}

U32String Translation::actionWith() const {
	return _("with");
}

U32String Translation::infoTextOffSoundOn() const {
	return _("Text off / Sound on");
}

U32String Translation::infoTextOnSoundOff() const {
	return _("Sound off / Text on");
}

U32String Translation::infoTextOn() const {
	return _("Text on");
}

U32String Translation::infoSoundOn() const {
	return _("Sound on");
}

U32String Translation::infoMusicOn() const {
	return _("Music on");
}

U32String Translation::infoMusicOff() const {
	return _("Music off");
}

U32String Translation::infoJoke(int index) const {
	return index < 0 || index >= ARRAYSIZE(kInfoJokes)
		? U32String()
		: _(kInfoJokes[index]);
}

}

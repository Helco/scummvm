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
#include "common/ustr.h"

namespace Edna {

class Translation {
public:
	Translation();

	// We reencode some of the strings into UTF8 because most text sources are natively UTF8
	// Reallocating them is much more work than reencoding these few static one.

	const char  *action(PlayerAction action) const;
	Common::U32String actionWith() const; // for "Use <item> *with* <target>"
	const char *dropTopic() const { return _dropTopic.c_str(); }

	inline const char *toggleMusic() const { return _toggleMusic.c_str(); }
	const char *toggleSound() const { return _toggleSound.c_str(); }
	const char *toggleSubtitles() const { return _toggleSubtitles.c_str(); }
	const char *textSpeed() const { return _textSpeed.c_str(); }
	Common::U32String infoTextOffSoundOn() const;
	Common::U32String infoTextOnSoundOff() const;
	Common::U32String infoTextOn() const;
	Common::U32String infoSoundOn() const;
	Common::U32String infoMusicOn() const;
	Common::U32String infoMusicOff() const;

	Common::U32String infoJoke(int index) const;

private:
	Common::String
		_actionNames[kPlayerActionCount],
		_dropTopic,
		_toggleMusic,
		_toggleSound,
		_toggleSubtitles,
		_textSpeed;
};

}

#endif // EDNA_TRANSLATION_H

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

#ifndef EDNA_OPTIONSMENU_H
#define EDNA_OPTIONSMENU_H

#include "edna/group/group.h"
#include "edna/sprite/button.h"
#include "edna/sprite/text.h"

namespace Edna {

class OptionsMenu : public Group {
public:
	void update() override;
	void render() override;

	void open();

protected:
	OptionsMenu();
	void toggleChecks();

	Config _previousConfig;
	Sprite
		_background,
		_textSpeedBg,
		_textSpeedSlider;
	AnimatedSprite
		_checkMusic,
		_checkSound,
		_checkSubtitles;
	Button
		_btnConfirm,
		_btnCancel;
	Text
		_textMusic,
		_textSound,
		_textSubtitles,
		_textTextSpeed;
	Common::ScopedPtr<IRenderedText> _infoText;
	Common::Point _infoTextPos;
	int _infoJokeIndex = 0;
	int16 _sliderMinX = 0, _sliderMaxX = 0;
	bool _isMovingSlider = false;
};

class InGameOptionsMenu : public OptionsMenu {
public:
	InGameOptionsMenu();
};

class StartOptionsMenu : public OptionsMenu {
public:
	StartOptionsMenu();

private:
	Sprite _infoTextBg;
};

}

#endif // EDNA_OPTIONSMENU_H

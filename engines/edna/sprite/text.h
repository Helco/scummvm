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

#ifndef EDNA_TEXT_H
#define EDNA_TEXT_H

#include "edna/sprite/sprite.h"

namespace Edna {

enum TextFlags {
	kTextWrapLines = 1 << 0,
	kTextMoveIntoScreen = 1 << 1
};

class Text final : public Sprite {
public:
	Text(Common::Point pos, FontKind font, const char *text, TextFlags flags);

	void render() override;
	void debugPrint() override;
	void setColor(FontKind font);

private:
	static constexpr uint kDebugTextSize = 32;
	Common::Array<Common::ScopedPtr<IRenderedText>> _lines;
	char _debugText[kDebugTextSize] = { };
};

}

#endif // EDNA_TEXT_H

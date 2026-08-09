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

#ifndef EDNA_CHOICELIST_H
#define EDNA_CHOICELIST_H

#include "edna/group/group.h"

namespace Edna {

class Game;
class Text;

class ChoiceList final : public Group {
public:
	ChoiceList(Game &game);

	inline ChoiceSetId setId() const { return _setId; }

	void update() override;
	void openSet(ChoiceSetId setId);
	void close();

private:
	Game &_game;
	ChoiceSetId _setId = 0;
	Text *_selected = nullptr;
};

}

#endif // EDNA_CHOICELIST_H

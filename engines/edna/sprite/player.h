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

#ifndef EDNA_PLAYER_H
#define EDNA_PLAYER_H

#include "edna/db.h"
#include "edna/sprite/character.h"

namespace Edna {

class Player final : public Character {
public:
	Player(Game &game, Common::Point startPos, const DB::Room &room);

	void debugPrint() override;
	int basePosX() const override;
	int basePosY() const override;
	int basePosY(int x) const override;
};

}

#endif // EDNA_PLAYER_H

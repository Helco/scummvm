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

#ifndef EDNA_GAME_H
#define EDNA_GAME_H

#include "common/span.h"

#include "edna/sprite/group.h"

namespace Edna {

// Everything except Intro inherits from Game

class GameBase {
public:
	GameBase(GameMode mode);
	virtual ~GameBase();

	inline GameMode gameMode() const { return _gameMode; }
	inline Common::Span<const Common::DisposablePtr<Group>> groups() const {
		return { _groups.data(), _groups.size() };
	}

	virtual void update();
	virtual void render();

protected:
	void add(Group *group, DisposeAfterUse::Flag dispose = DisposeAfterUse::YES);

private:
	const GameMode _gameMode;
	Common::Array<Common::DisposablePtr<Group>> _groups;
};

class Game : public GameBase {
public:
	Game(GameMode mode, RoomId roomId);

	inline RoomId roomId() const { return _roomId; }

private:
	const RoomId _roomId;
};

}

#endif // EDNA_GAME_H

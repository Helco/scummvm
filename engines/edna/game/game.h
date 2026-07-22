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

#include "edna/script.h"
#include "edna/sprite/group.h"

namespace Edna {

class Player;

// Everything except Intro inherits from Game

class GameBase {
public:
	GameBase(Common::ScopedPtr<GameBase> &myPtr, GameMode mode);
	virtual ~GameBase();

	inline GameMode gameMode() const { return _gameMode; }
	inline Common::Span<const Common::DisposablePtr<Group>> groups() const {
		return { _groups.data(), _groups.size() };
	}

	virtual void update();
	virtual void render();

protected:
	void add(Group *group, DisposeAfterUse::Flag dispose = DisposeAfterUse::NO);

private:
	const GameMode _gameMode;
	Common::Array<Common::DisposablePtr<Group>> _groups;
};

class Game : public GameBase {
public:
	Game(Common::ScopedPtr<GameBase> &myPtr, GameMode mode, RoomId roomId);

	inline RoomId roomId() const { return _roomId; }
	inline Script &script() { return _script; }
	inline Player &player() { assert(_player != nullptr); return *_player; }
	inline Group &background() { return _background; }
	inline Group &bgObjects() { return _bgObjects; }
	inline Group &objects() { return _objects; }
	inline Group &texts() { return _texts; }
	inline Group &gui() { return _gui; }

	void update() override;
	void render() override;
	Sprite *objectById(RoomObjectId id) const;

	void fade(byte color, float target, uint32 duration);

protected:
	void initBackground(const char *background);
	void initTimer(TimerId timerId);
	virtual void initGroups();
	void initGroups(Group *specialObjects, Group *specialGui);
	void initPlayer();
	void initObjects();

	void updateFade();
	bool updateScript();

private:
	const RoomId _roomId;
	Script _script;
	Player *_player = nullptr;
	Group _background;
	Group _bgObjects;
	ObjectGroup _objects;
	Group _texts;
	Group _gui;
	// missing due to custom type: comment, mainMenu, choiceList

	bool _pendingTimerInvoke = false;
	ScriptId _timerScript;
	Timer _timer;

	struct {
		bool _active = false;
		byte _color = 0; ///< there is only fades to black or white, so this is okay
		float _current = 0, _speed = 0;
	} _fade;
};

}

#endif // EDNA_GAME_H

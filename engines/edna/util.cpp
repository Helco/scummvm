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

#include "edna/edna.h"
#include "edna/util.h"

#include "common/intrinsics.h"

namespace Edna {

bool Timer::update() {
    if (!_active)
        return false;
	_curDelay += g_engine->getElapsed();
    if (_curDelay < _delay)
        return false;
    _curDelay -= _delay;
    return true;
}

void Timer::reset() {
	_curDelay = 0;
}

void Timer::toggle(bool active) {
    _active = active;
    reset();
}

static constexpr const char *const DirectionNames[] = {
	"None",
	"Up",
	"Down",
	"Left",
	"Right"
};
const char *directionToString(Direction dir) {
	int index = 1 + (int)dir;
	if (index < 0 || index >= ARRAYSIZE(DirectionNames))
		return "<unknown>";
	return DirectionNames[index];
}

static constexpr const char *const GameModeNames[] = {
	"None",
	"StartMenu",
	"EdnaStd",
	"Harvey",
	"EdnaGirl",
	"ScriptOnClick",
	"DragScript",
	"Zen",
	"MainMenu",
	"Intro"
};

const char *gameModeToString(GameMode mode) {
	int index = 1 + (int)mode;
	if (index < 0 || index >= ARRAYSIZE(GameModeNames))
		return "<unknown>";
	return GameModeNames[index];
}

static constexpr const char *const PlayerActionNames[] = {
	"None",
	"Look",
	"Use",
	"Pick",
	"Talk",
	"Walk",
	"ToHarvey",
	"ToEdna",
	"WhatIs",
	"TalkAbout"
};

const char *playerActionToString(PlayerAction action) {
	int index = (int)action;
	if (index < 0 || index >= ARRAYSIZE(PlayerActionNames))
		return "<unknown>";
	return PlayerActionNames[index];
}

static constexpr const char *const FontKindNames[] = {
	"EdnaFont",
	"HarveyFont",
	"NscFontRot",
	"NscFontGelb",
	"NscFontOrange",
	"NscFontGreygreen",
	"NscFontBlau",
	"NscFontGrau",
	"NscFontHellgelb",
	"NscFontLind",
	"NscFontStahlblau",
	"NscFontWeiss",
	"TestFont",
	"ActiveFont",
	"InactiveFont",
	"MenuFont",
	"MenuFont2"
};

const char *fontKindToString(FontKind kind) {
	int index = (int)kind;
	if (index < 0 || index >= ARRAYSIZE(FontKindNames))
		return "<unknown>";
	return FontKindNames[index];
}

bool parseDirection(const char *text, Direction &value) {
	assert(text != nullptr);
	switch (tolower(text[0])) {
	case '0':
	case 'n':
		value = Direction::Up;
		break;
	case '1':
	case 's':
		value = Direction::Down;
		break;
	case 'o': // this is a default case and the developers apparently could not decide
	case 'e': // whether to use english or german compass directions or enum numbers...
	case '3':
		value = Direction::Right;
		break;
	case '2':
	case 'w':
		value =  Direction::Left;
		break;
	default:
		return false;
	}
	return text[1] == '\0';
}

bool parsePlayerAction(const char *text, PlayerAction &value) {
	assert(text != nullptr);
	switch (tolower(text[0])) {
	case '0':
		value = PlayerAction::None;
		break;
	case 'a':
	case 'l':
		value = PlayerAction::Look;
		break;
	case 'b':
	case 'u':
		value = PlayerAction::Use;
		break;
	case 'n':
	case 'p':
		value = PlayerAction::Pick;
		break;
	case 'r':
	case 't':
		value = PlayerAction::Talk;
		break;
	// Walk has no stored value
	default:
		return false;
	}
	return text[1] == '\0';
}

int compare(const TwoKey &a, const TwoKey &b) {
	if (a.first == b.first) {
		return a.second == b.second ? 0
			: a.second < b.second ? -1 : 1;
	} else
		return a.first < b.first ? -1 : 1;
}

bool less(const TwoKey &a, const TwoKey &b) {
	return a.first < b.first || (a.first == b.first && a.second < b.second);
}

bool PlayerCommand::operator==(const PlayerCommand &other) const {
	return _action == other._action &&
		_target == other._target &&
		_item == other._target &&
		_targetPos == other._targetPos &&
		_isComplete == other._isComplete;
}

bool PlayerCommand::operator!=(const PlayerCommand &other) const {
	return !(*this == other);
}

// the original engine converts the ID to a string then reparses a substring
// we can avoid that by just using the convention as is (every ID is 6 digits)
static void decomposeRoomId(RoomId roomId, uint32 &v, uint32 &q) {
	assert(roomId >= 100000);
	v = roomId / 100;
	q = roomId % 100;
	assert(q < 10000);
}

PastRoomIds PastRoomIds::fromHarveyRoom(RoomId harveyRoom) {
	uint32 v, q;
	decomposeRoomId(harveyRoom, v, q);
	PastRoomIds ids;
	ids._harveyRoom = harveyRoom;
	ids._ednaRoom = harveyRoom + 50;
	ids._ednaId = v * 10000 + 9900 + q;
	ids._harveyId = v * 10000 + 9900 + q + 50;
	return ids;
}

PastRoomIds PastRoomIds::fromEdnaRoom(RoomId ednaRoom) {
	assert(ednaRoom >= 100050);
	return fromHarveyRoom(ednaRoom - 50);
}

}

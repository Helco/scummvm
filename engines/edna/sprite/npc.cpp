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

#include "edna/sprite/npc.h"

#include "gui/debugger.h"

using namespace Common;

namespace Edna {

Npc::Npc(
	Game &game,
	const DB::NPC &npc,
	RoomInteractionId interactionId,
	Point startPos,
	Point baseLineStart,
	Point baseLineEnd)
	: Character(game, startPos, npc._charAnimSet, npc._hspeed, npc._vspeed, npc._baseYAtZeroScale, npc._baseYAtFullScale)
	, InteractableRoomObject(interactionId)
	, _name(npc._name)
	, _baseLineStart(baseLineStart)
	, _baseLineEnd(baseLineEnd) {
	_direction = Direction::Left;
	_talkFont = _thinkFont = npc._font;
}

void Npc::debugPrint() {
	g_engine->getDebugger()->debugPrintf("NPC \"%s\" (%s)\n", _name, stateToString());
}

int32 Npc::basePosX() const {
	return pos().x + size().x / 2;
}

int32 Npc::basePosY() const {
	return pos().y + size().y;
}

int32 Npc::basePosY(int x) const {
	auto delta = _baseLineEnd - _baseLineStart;
	float ratio = delta.y / (float)delta.x;
	return (int)(_baseLineStart.y + ratio * (x - _baseLineStart.x));
}


}

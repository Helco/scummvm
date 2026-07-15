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

#include "edna/console.h"
#include "edna/db.h"
#include "edna/edna.h"

namespace Edna {

Console::Console() : GUI::Debugger() {
	registerCmd("validate", WRAP_METHOD(Console, cmdValidate));
	registerCmd("room", WRAP_METHOD(Console, cmdRoom));
}

Console::~Console() {
}

bool Console::cmdValidate(int argc, const char **argv) {
	uint32 errors = g_engine->db().validate();
	if (errors == 0)
		debugPrintf("Validation successful!\n");
	else
		debugPrintf("Validation failed (%u errors)!\n\n", errors);
	return true;
}

bool Console::cmdRoom(int argc, const char **argv) {
	RoomId roomId;
	if (argc != 1 && argc != 2) {
		debugPrintf("usage: room [id]");
		return true;
	} else if (argc == 1) {
		debugPrintf("current room is not supported yet\n");
		return true;
	} else {
		char *end = nullptr;
		roomId = (RoomId)strtoul(argv[1], &end, 10);
		if (end == nullptr || *end != '\0') {
			debugPrintf("room id has to be an unsigned integer\n");
			return true;
		}
	}

	// Room details
	auto &db = g_engine->db();
	DB::Room room = db.room(roomId, false);
	auto objects = db.roomObjectsByRoom(roomId, false);
	if (room._id != roomId) {
		debugPrintf("Unknown room and cannot look for partial room IDs yet\n");
		return true;
	}
	debugPrintf("Room %u %s (%s)\n", room._id, room._name, gameModeToString(room._gameMode));
	debugPrintf("  Background: %s\n  Music: %s\n  Speeds (V/H): %f, %f\n  BaseY: %f -> %f\n",
		room._background, room._music, room._vspeed, room._hspeed, room._baseYAtZeroScale, room._baseYAtFullScale);
	debugPrintf("  CharAnimSet: %u\n  WalkableAreaId: %u\n  Timer: %u\n  Objects: %u\n",
		room._charAnimSet, room._walkAreaId, room._timer, objects.size());
	if (objects.size() == 0)
		return true;

	// Object summary
	debugPrintf("  ID        Active Name                    Flags Image\n");
	char flags[6] = "IDTtN";
	for (auto objectId : objects) {
		DB::RoomObject obj = db.roomObject(objectId, false);
		if (obj._id != objectId) {
			debugPrintf("  %10d       <invalid>\n", objectId);
			continue;
		}
		flags[0] = obj._toInteraction ? 'I' : ' ';
		flags[1] = obj._toDisplay ? 'D' : ' ';
		flags[2] = obj._toTopicId ? 'T' : ' ';
		flags[3] = obj._toTopicObject ? 't' : ' ';
		flags[4] = obj._toNPC ? 'N' : ' ';
		debugPrintf("  %9d %-6s%-24s%s %s\n", obj._id, obj._active ? "true" : "false", obj._name, flags, obj._image);
	}
	debugPrintf("\n");
	return true;
}

} // End of namespace Edna

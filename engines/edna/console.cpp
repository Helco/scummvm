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
#include "edna/game/game.h"
#include "edna/sprite/sprite.h"
#include "edna/pathfinder.h"

#include "common/file.h"
#include "image/png.h"

using namespace Common;

namespace Edna {

Console::Console()
	: GUI::Debugger()
	, _breakpoints(compare) {
	registerCmd("validate", WRAP_METHOD(Console, cmdValidate));
	registerCmd("room", WRAP_METHOD(Console, cmdRoom));
	registerCmd("sprites", WRAP_METHOD(Console, cmdSprites));
	registerCmd("object", WRAP_METHOD(Console, cmdObject));
	registerCmd("script", WRAP_METHOD(Console, cmdScript));
	registerCmd("stop", WRAP_METHOD(Console, cmdStop));
	registerCmd("eval", WRAP_METHOD(Console, cmdEval));
	registerCmd("e", WRAP_METHOD(Console, cmdEval));
	registerCmd("br", WRAP_METHOD(Console, cmdBreakpoint));
	registerCmd("delbr", WRAP_METHOD(Console, cmdDelBreakpoint));
	registerCmd("dumpFloor", WRAP_METHOD(Console, cmdDumpFloor));

	registerVar("waitDiv", &_waitDivider);
	registerVar("debugSprites", &_debugSprites);
	registerVar("debugFloor", &_debugFloor);
}

Console::~Console() {
}

Game *Console::getGame() {
	Game *game = dynamic_cast<Game *>(&g_engine->game());
	if (game == nullptr)
		debugPrintf("Current game (%s) is not a room game\n", gameModeToString(g_engine->game().gameMode()));
	return game;
}

bool Console::tryParseUint(const char *arg, uint32 &value, const char *context) {
	char *end = nullptr;
	value = (uint32)strtoul(arg, &end, 10);
	if (end == nullptr || *end != '\0') {
		debugPrintf("%s has to be an unsigned integer\n", context);
		return false;
	}
	return true;
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
		Game *game = getGame();
		if (game == nullptr)
			return true;
		roomId = game->roomId();
	} else if (!tryParseUint(argv[1], roomId, "Room id"))
		return true;

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
	debugPrintf("  ID        Active Name                    Flags  Image\n");
	char flags[7] = "IDTtNE";
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
		flags[5] = obj._toInteraction && g_engine->db().roomInteraction(obj._toInteraction, false)._toExit ? 'E' : ' ';
		debugPrintf("  %9d %-6s%-24s%s %s\n", obj._id, obj._active ? "true" : "false", obj._name, flags, obj._image);
	}
	debugPrintf("\n");
	return true;
}

bool Console::cmdSprites(int argc, const char **argv) {
	GameBase &game = g_engine->game();
	debugPrintf("  ID         Flags PosX   PosY   Width  Height \n");
	for (const auto &group : game.groups()) {
		const auto sprites = group->sprites();
		debugPrintf("%s \"%s\" (%u)\n", group->active() ? "ACTIVE" : "INACTIVE", group->name(), sprites.size());
		for (const auto &sprite : sprites) {
			char flags[4] = "AIO";
			flags[0] = sprite->active() ? 'A' : ' ';
			flags[1] = sprite->immutable() ? 'I' : ' ';
			flags[2] = ' ';

			// Is this an overlayed room object?
			const auto object = g_engine->db().roomObject(sprite->id(), false);
			if (object._id == sprite->id() && (
				g_engine->db()._roomObjects._overlay.contains(object._id) ||
				g_engine->db()._roomInteractions._overlay.contains(object._toInteraction)))
				flags[2] = 'O';

			debugPrintf("  %10u  %s  %6d %6d %6d %6d ",
				sprite->id(), flags, sprite->pos().x, sprite->pos().y, sprite->size().x, sprite->size().y);
			sprite->debugPrint();
		}
	}
	return true;
}

bool Console::cmdObject(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: object <id>\n");
		return true;
	}
	RoomObjectId objId;
	if (!tryParseUint(argv[1], objId, "object id"))
		return true;
	const DB::RoomObject obj = g_engine->db().roomObject(objId, false);
	if (obj._id != objId) {
		debugPrintf("No such room object: %u\n", objId);
		return true;
	}

	// Base properties
	DB::RoomObject origObj;
	if (g_engine->db()._roomObjects._overlay.contains(objId))
		origObj = g_engine->db()._roomObjects._map[objId];
	debugPrintf("Object %u\n", objId);
	debugPrintf("     Property                  Current                 Original\n");
	debugPrintf("O   Obj. Name %24s\n", obj._name);
	debugPrintf("O        Room %24u\n", obj._room);
#define EDNA_OVERLAY(cond, part1, part2) (cond) ? part1 part2 "\n" : part1 "\n"
	debugPrintf(EDNA_OVERLAY(origObj._id == objId, "O      Active %24s", " %24s"),
		obj._active ? "true" : "false", origObj._active ? "true" : "false");
	debugPrintf(EDNA_OVERLAY(origObj._id == objId, "O    Position        %8d,%8d", "        %8d,%8d\n"),
		obj._pos.x, obj._pos.y, origObj._pos.x, origObj._pos.y);
	debugPrintf("O       Image %24s\n", obj._image);

	const DB::RoomObjectDisplay display = g_engine->db().roomObjectDisplay(obj._toDisplay, false);
	if (display._object == objId) {
		const auto animation = g_engine->db().animation(display._animation, false);
		debugPrintf("D     Display %24u\n", display._id);
		debugPrintf("D   Animation %24u (%s)\n", display._animation, animation._name);
		debugPrintf("D    BaseLine   (%3d,%3d) -> (%3d,%3d)\n",
			display._baseLineStart.x, display._baseLineStart.y, display._baseLineEnd.x, display._baseLineEnd.y);
	}

	const DB::RoomInteraction inter = g_engine->db().roomInteraction(obj._toInteraction, false);
	if (inter._object == objId) {
		DB::RoomInteraction origInter;
		if (g_engine->db()._roomInteractions._overlay.contains(obj._toInteraction))
			origInter = g_engine->db()._roomInteractions._overlay[obj._toInteraction];
		debugPrintf("I Interaction %24u\n", inter._id);
		debugPrintf("I Inter. Name %24s\n", inter._name);
		debugPrintf("I Inter.  Pos        %8d,%8d\n", inter._walkTo.x, inter._walkTo.y);
		debugPrintf("I Inter.  Dir %24s\n", directionToString(inter._lookDirection));
		debugPrintf("I Def. Action %24s\n", playerActionToString(inter._defaultAction));
		debugPrintf(EDNA_OVERLAY(inter._id == origInter._id, "I Look Script %24u", " %24u\n"),
			inter._lookScript, origInter._lookScript);
		debugPrintf(EDNA_OVERLAY(inter._id == origInter._id, "I Pick Script %24u", " %24u\n"),
			inter._pickScript, origInter._pickScript);
		debugPrintf(EDNA_OVERLAY(inter._id == origInter._id, "I Talk Script %24u", " %24u\n"),
			inter._talkScript, origInter._talkScript);
		debugPrintf(EDNA_OVERLAY(inter._id == origInter._id, "I  Use Script %24u", " %24u\n"),
			inter._useScript, origInter._useScript);
	}

	const DB::RoomExit exit = g_engine->db().roomExit(inter._toExit, false);
	if (inter._object == objId && exit._interaction == inter._id) {
		debugPrintf("E        Exit %24u\n", exit._id);
		debugPrintf("E      Target %24u (%s)\n", exit._target, g_engine->db().room(exit._target, false)._name);
		debugPrintf("E     Walk-In        %8d,%8d\n", exit._walkIn.x, exit._walkIn.y);
		debugPrintf("E Walk-In-Dir %24s\n", directionToString(exit._lookDirection));
	}

	const DB::NPC npc = g_engine->db().npc(obj._toNPC, false);
	if (npc._object == objId) {
		debugPrintf("N         NPC %24u\n", npc._id);
		debugPrintf("N        Name %24s\n", npc._name);
		debugPrintf("N CharAnimSet %24u\n", npc._charAnimSet);
		debugPrintf("N        Font %24s\n", fontKindToString(npc._font));
		debugPrintf("N       Speed        %8f,%8f\n", npc._hspeed, npc._vspeed);
		debugPrintf("N     Scale-Y     %8f -> %8f\n", npc._baseYAtZeroScale, npc._baseYAtFullScale);
	}

	if (obj._toTopicId != 0)
		debugPrintf("T       Topic %24u (%s)\n", obj._toTopicId, g_engine->db().topic(obj._toTopicId)._name);
	if (obj._toTopicObject != 0)
		debugPrintf("T   Topic Obj %24u (%s)\n", obj._toTopicObject, g_engine->db().topic(obj._toTopicObject)._name);
	return true;
}

bool Console::cmdScript(int argc, const char **argv) {
	uint32 scriptId;
	uint32 markLine = UINT32_MAX;
	if (argc == 1) {
		Game *game = getGame();
		if (game == nullptr)
			return true;
		if (!game->script().isScriptRunning()) {
			debugPrintf("No script is currently running\n");
			return true;
		}
		scriptId = game->script().scriptId();
		markLine = game->script().scriptLine();
	} else if (argc == 2) {
		if (!tryParseUint(argv[1], scriptId, "Script id"))
			return true;
	} else {
		debugPrintf("usage: script [<script id>]\n");
		return true;
	}
	auto lines = g_engine->db().script(scriptId, false);
	if (lines.size() == 0) {
		debugPrintf("Invalid script id\n");
		return true;
	}

	debugPrintf("Script %u:\n", scriptId);
	for (const auto &line : lines) {
		debugPrintf("%c %3d: ", (line._line == markLine ? '>' : ' '), line._line);
		line._command.debugPrintConsole();
	}
	return true;
}

bool Console::cmdStop(int argc, const char **argv) {
	Game *game = getGame();
	if (game == nullptr)
		return true;
	if (!game->script().isScriptRunning()) {
		debugPrintf("No script is currently running\n");
		return true;
	}
	game->script().stop();
	return true;
}

bool Console::cmdEval(int argc, const char **argv) {
	Game *game = getGame();
	if (game == nullptr)
		return true;
	if (argc != 2) {
		debugPrintf("usage: eval <script line>\n");
		return true;
	}
	Script &script = game->script();
	if (script.isScriptRunning() || script.isPerforming()) {
		debugPrintf("Cannot evaluate script command, another script is running or performing\n");
		return true;
	}

	char *commandStr = scumm_strdup(argv[1]);
	ScriptCommand command(commandStr);
	if (command._handler == nullptr) {
		free(commandStr);
		debugPrintf("Could not parse script line\n");
		return true;
	}

	bool result = (script.*command._handler)(command);
	free(commandStr);
	return result;
}

bool Console::cmdRun(int argc, const char **argv) {
	Game *game = getGame();
	if (game == nullptr)
		return true;
	if (argc != 2 && argc != 3) {
		debugPrintf("usage: run <script> [<line>]\n");
		return true;
	}
	ScriptId scriptId = 0;
	uint32 line = 1;
	if (!tryParseUint(argv[1], scriptId, "script id"))
		return true;
	if (argc == 3 && !tryParseUint(argv[2], line, "script line"))
		return true;

	Script &script = game->script();
	if (script.isScriptRunning() || script.isPerforming()) {
		debugPrintf("Cannot evaluate script command, another script is running or performing\n");
		return true;
	}
	script.runNew(scriptId, line);
	return false;
}

bool Console::cmdBreakpoint(int argc, const char **argv) {
	if (argc < 1 || argc > 3) {
		debugPrintf("usage: br [<script> [<line>]]\n");
		return true;
	}
	if (argc == 1) {
		printBreakpointList();
		return true;
	}

	ScriptId script = 0;
	uint32 line = 1;
	if (!tryParseUint(argv[1], script, "script id"))
		return true;
	if (argc == 3 && !tryParseUint(argv[2], line, "script line"))
		return true;
	const auto scriptLines = g_engine->db().script(script, false);
	const auto itLine = find_if(scriptLines.begin(), scriptLines.end(),
		[line](const DB::ScriptLine &l) { return l._line == line; });
	if (itLine == scriptLines.end()) {
		debugPrintf("Invalid script line: %u %u\n", script, line);
		return true;
	}
	const auto itBreakpoint = getBreakpoint(script, line);
	if (itBreakpoint != _breakpoints.end()) {
		debugPrintf("Breakpoint already set\n");
		return true;
	}

	_breakpoints.insert({ script, line });
	debugPrintf("Breakpoint #%u set at %u %u\n", _breakpoints.size() - 1, script, line);
	return true;
}

bool Console::cmdDelBreakpoint(int argc, const char **argv) {
	if (argc < 1 || argc > 3) {
		debugPrintf("usages:\n  delbr [*]\n  delbr #<breakpoint-index>\n  delbr <script> [<line>]\n");
		return true;
	}
	if (argc == 1) {
		printBreakpointList();
		return true;
	}
	if (argv[1][0] == '*') {
		debugPrintf("All %u breakpoints deleted\n", _breakpoints.size());
		_breakpoints.clear();
		return true;
	}
	if (argv[1][0] == '#') {
		uint32 idx = 0;
		if (!tryParseUint(argv[1] + 1, idx, "breakpoint index"))
			return true;
		if (idx >= _breakpoints.size()) {
			debugPrintf("Invalid breakpoint index %u\n", idx);
			return true;
		}
		_breakpoints.remove_at(idx);
		return true;
	}

	ScriptId script = 0;
	uint32 line = 1;
	if (!tryParseUint(argv[1], script, "script id"))
		return true;
	if (argc == 3 && !tryParseUint(argv[2], line, "script line"))
		return true;
	auto it = getBreakpoint(script, line);
	if (it == _breakpoints.end())
		debugPrintf("No breakpoint set at %u %u\n", script, line);
	else
		_breakpoints.remove_at(it - _breakpoints.begin());
	return true;
}

Console::BreakpointList::const_iterator Console::getBreakpoint(ScriptId scriptId, uint32 line) const {
	auto it = lowerBound(_breakpoints.begin(), _breakpoints.end(), TwoKey(scriptId, line), less);
	if (it != _breakpoints.end() && (it->first != scriptId || it->second != line))
		it = _breakpoints.end();
	return it;
}

void Console::printBreakpointList() {
	if (_breakpoints.empty()) {
		debugPrintf("No breakpoints set\n");
		return;
	}
	debugPrintf("%u breakpoints:\n", _breakpoints.size());
	for (uint32 i = 0; i < _breakpoints.size(); i++) {
		const TwoKey &point = *(_breakpoints.begin() + i);
		debugPrintf("  #%-3u: %10u %3u\n", i, point.first, point.second);
	}
}

bool Console::hasBreakpoint(ScriptId scriptId, uint32 line) const {
	return getBreakpoint(scriptId, line) != _breakpoints.end();
}

bool Console::cmdDumpFloor(int argc, const char **argv) {
	static const byte colors[] = { 0, 0, 0, 0xff, 0xff, 0xff };
	DumpFile dmpFile;
	if (!dmpFile.open("floor.png")) {
		debugPrintf("Could not open floor.png\n");
		return true;
	}
	Graphics::Surface surface;
	surface.pitch = surface.w = kScreenHeight;
	surface.h = kScreenWidth;
	surface.format = Graphics::PixelFormat::createFormatCLUT8();
	surface.setPixels(const_cast<byte *>(g_engine->pathFinder().map().data()));
	if (!Image::writePNG(dmpFile, surface, colors, 2))
		debugPrintf("Could not write floor.png\n");
	else
		debugPrintf("Saved floor to floor.png\n");
	return true;
}

} // End of namespace Edna

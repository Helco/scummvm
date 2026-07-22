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

using namespace Common;

namespace Edna {

Console::Console()
	: GUI::Debugger()
	, _breakpoints(compare) {
	registerCmd("validate", WRAP_METHOD(Console, cmdValidate));
	registerCmd("room", WRAP_METHOD(Console, cmdRoom));
	registerCmd("sprites", WRAP_METHOD(Console, cmdSprites));
	registerCmd("script", WRAP_METHOD(Console, cmdScript));
	registerCmd("eval", WRAP_METHOD(Console, cmdEval));
	registerCmd("e", WRAP_METHOD(Console, cmdEval));
	registerCmd("br", WRAP_METHOD(Console, cmdBreakpoint));
	registerCmd("delbr", WRAP_METHOD(Console, cmdDelBreakpoint));
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

bool Console::cmdSprites(int argc, const char **argv) {
	GameBase &game = g_engine->game();
	debugPrintf("  ID         Flags PosX   PosY   Width  Height \n");
	for (const auto &group : game.groups()) {
		const auto sprites = group->sprites();
		debugPrintf("%s \"%s\" (%u)\n", group->active() ? "ACTIVE" : "INACTIVE", group->name(), sprites.size());
		for (const auto &sprite : sprites) {
			char flags[3] = "AI";
			flags[0] = sprite->active() ? 'A' : ' ';
			flags[1] = sprite->immutable() ? 'I' : ' ';
			debugPrintf("  %10u  %s   %6d %6d %6d %6d ",
				sprite->id(), flags, sprite->pos().x, sprite->pos().y, sprite->size().x, sprite->size().y);
			sprite->debugPrint();
		}
	}
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
	script.runNewScript(scriptId, line);
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

} // End of namespace Edna

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

#include "topgun/console.h"
#include "topgun/topgun.h"

namespace TopGun {

extern const char *scriptPointTypeNames[];

Console::Console(TopGunEngine *engine) :
	GUI::Debugger(),
	_engine(engine),
	_scriptDebugger(engine->_script->getDebugger()) {
	registerCmd("gameInfo", WRAP_METHOD(Console, Cmd_gameInfo));
	registerCmd("trace", WRAP_METHOD(Console, Cmd_addPoint));
	registerCmd("break", WRAP_METHOD(Console, Cmd_addPoint));
	registerCmd("delete", WRAP_METHOD(Console, Cmd_removePoint));
	registerCmd("delete-all", WRAP_METHOD(Console, Cmd_removeAllPoints));
	registerCmd("continue", WRAP_METHOD(Console, Cmd_continue));
	registerCmd("step", WRAP_METHOD(Console, Cmd_step));
	registerCmd("stepOver", WRAP_METHOD(Console, Cmd_stepOver));
	registerCmd("stepOut", WRAP_METHOD(Console, Cmd_stepOut));
	registerCmd("list-breaks", WRAP_METHOD(Console, Cmd_listPoints));
	registerCmd("stacktrace", WRAP_METHOD(Console, Cmd_stacktrace));
	registerCmd("scenestack", WRAP_METHOD(Console, Cmd_scenestack));
	registerCmd("localVars", WRAP_METHOD(Console, Cmd_localVars));
	registerCmd("sceneVars", WRAP_METHOD(Console, Cmd_globalVars));
	registerCmd("systemVars", WRAP_METHOD(Console, Cmd_globalVars));
	registerCmd("dynString", WRAP_METHOD(Console, Cmd_dynStrings));
	registerCmd("dynStrings", WRAP_METHOD(Console, Cmd_dynStrings));
	registerCmd("listSprites", WRAP_METHOD(Console, Cmd_listSprites));
	registerCmd("spriteInfo", WRAP_METHOD(Console, Cmd_spriteInfo));

	registerVar("drawSpriteIDs", &engine->_spriteCtx->_debugDrawSpriteIDs);
}

Console::~Console() {
}

bool Console::Cmd_gameInfo(int argc, const char **argv) {
	auto gameDesc = _engine->getGameDesc();
	debugPrintf("%s %s %s %s\n",
		gameDesc->_baseDescription.gameId,
		gameDesc->_baseDescription.extra == nullptr ? "" : gameDesc->_baseDescription.extra,
		Common::getLanguageCode(gameDesc->_baseDescription.language),
		Common::getPlatformCode(gameDesc->_baseDescription.platform));
	debugPrintf("Scene/system variables: %d/%d\n", (int)gameDesc->_sceneVarCount, (int)gameDesc->_systemVarCount);
	return true;
}

bool Console::Cmd_addPoint(int argc, const char **argv) {
	if (argc < 2 || argc > 4) {
usage:
		debugPrintf("usage: %s <type> [index] [offset]\n", argv[0]);
		debugPrintf("types: ");
		for (const char **type = scriptPointTypeNames; *type; type++)
			debugPrintf(" %s", *type);
		debugPrintf("\n");
		return true;
	}
	bool breaks = argv[0][0] == 'b';
	ScriptPointType type = ScriptPointType::kInvalid;
	for (int i = 0; scriptPointTypeNames[i]; i++) {
		if (!scumm_stricmp(scriptPointTypeNames[i], argv[1])) {
			type = (ScriptPointType)i;
			break;
		}
	}
	if (type == ScriptPointType::kInvalid) {
		debugPrintf("Invalid %s type\n", argv[0]);
		goto usage;
	}
	if ((argc < 3 && ScriptDebugger::pointTypeNeedsIndex(type)) ||
		(argc < 4 && ScriptDebugger::pointTypeNeedsOffset(type)))
		goto usage;

	uint32 index = 0, offset = 0;
	if (argc > 2)
		index = Common::String(argv[2]).asUint64Ext();
	if (argc > 3)
		offset = Common::String(argv[3]).asUint64Ext();

	const auto id = _scriptDebugger->addPoint(type, breaks, index, offset);
	debugPrintf("%s %d created\n", argv[0], id);
	return true;
}

bool Console::Cmd_removePoint(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: delete <id>\n");
		return true;
	}

	uint32 id = Common::String(argv[1]).asUint64Ext();
	if (_scriptDebugger->removePoint(id))
		debugPrintf("Point %d deleted\n", id);
	else
		debugPrintf("Invalid point id %d\n", id);
	return true;
}

bool Console::Cmd_removeAllPoints(int argc, const char **argv) {
	_scriptDebugger->removeAllPoints();
	return true;
}

bool Console::Cmd_continue(int argc, const char **argv) {
	_scriptDebugger->runContinue();
	return true;
}

bool Console::Cmd_step(int argc, const char **argv) {
	_scriptDebugger->runStep();
	return true;
}

bool Console::Cmd_stepOver(int argc, const char **argv) {
	_scriptDebugger->runStepOver();
	return true;
}

bool Console::Cmd_stepOut(int argc, const char **argv) {
	_scriptDebugger->runStepOut();
	return true;
}

bool Console::Cmd_listPoints(int argc, const char **argv) {
	_scriptDebugger->printAllPoints();
	return true;
}

bool Console::Cmd_stacktrace(int argc, const char **argv) {
	_scriptDebugger->printStacktrace();
	return true;
}

bool Console::Cmd_scenestack(int argc, const char **argv) {
	_engine->printSceneStack();
	return true;
}

bool Console::Cmd_localVars(int argc, const char **argv) {
	if (argc == 1)
		_scriptDebugger->printLocalScope();
	else if (argc == 2) {
		uint32 index = Common::String(argv[1]).asUint64Ext();
		_scriptDebugger->printLocalScope(index);
	}
	else
		debugPrintf("usage: localVars [scope index]\n");
	return true;
}

bool Console::Cmd_globalVars(int argc, const char **argv) {
	if (argc != 2 && argc != 3) { // let's not print 5001 variables
		debugPrintf("usage: %s <index> [count]\n", argv[0]);
		return true;
	}
	uint32 index = Common::String(argv[1]).asUint64Ext();
	uint32 count = 1;
	if (argc == 3)
		count = Common::String(argv[2]).asUint64Ext();
	if (argv[0][1] == 'y') // systemVars
		_scriptDebugger->printSystemVariables(index, count);
	else // sceneVars
		_scriptDebugger->printSceneVariables(index, count);
	return true;
}

bool Console::Cmd_dynStrings(int argc, const char **argv) {
	if (argc == 1)
		_scriptDebugger->printDynamicStrings();
	else if (argc == 2 || argc == 3) {
		uint32 index = Common::String(argv[1]).asUint64Ext();
		uint32 count = 1;
		if (argc == 3)
			count = Common::String(argv[2]).asUint64Ext();
		_scriptDebugger->printDynamicStrings(index, count);
	}
	else
		debugPrintf("usage: %s [index] [count]\n", argv[0]);
	return true;
}

bool Console::Cmd_listSprites(int argc, const char **argv) {
	_engine->getSpriteCtx()->printSprites();
	return true;
}

bool Console::Cmd_spriteInfo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("usage: spriteInfo <index>\n");
		return true;
	}
	uint32 index = Common::String(argv[1]).asUint64Ext();
	auto type = _engine->getResourceType(index);
	if (type != ResourceType::kSprite) {
		debugPrintf("Resource %d is not a sprite but a %d\n", index, type);
		return true;
	}
	if (!_engine->isResourceLoaded(index)) {
		debugPrintf("Sprite %d is not loaded\n", index);
		return true;
	}
	auto sprite = _engine->loadResource<Sprite>(index);
	sprite->printInfo();
	return true;
}

} // End of namespace Topgun

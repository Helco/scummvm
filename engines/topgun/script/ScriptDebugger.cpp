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

#include "topgun/topgun.h"
#include "gui/gui-manager.h"

namespace TopGun {

const char* scriptPointTypeNames[] {
	"script",
	"procedure",
	"variable-read",
	"variable-write",
	"variable-access",
	"resource-load",
	"resource-access",
	"scene-changing",
	"scene-changed",
	nullptr
};

const char *scriptCallTypeNames[]{
	"root",
	"calc",
	"proc",
	nullptr
};

bool ScriptDebugger::pointTypeNeedsIndex(ScriptPointType type) {
	return type != ScriptPointType::kSceneChanging && type != ScriptPointType::kSceneChanged;
}

bool ScriptDebugger::pointTypeNeedsOffset(ScriptPointType type) {
	return type == ScriptPointType::kScript;
}

bool ScriptTracePoint::appliesTo(const ScriptCallStackEntry &call) {
	switch (_type) {
	case ScriptPointType::kScript:
		if (call._type != ScriptCallType::kRoot && call._type != ScriptCallType::kCalc)
			return false;
		break;
	case ScriptPointType::kProcedure:
		if (call._type != ScriptCallType::kProcedure)
			return false;
		break;
	default: return false;
	}
	return _index == call._index && _offset == call._offset;
}

ScriptDebugger::ScriptDebugger(TopGunEngine *engine) :
	_engine(engine) {
}

uint32 ScriptDebugger::addPoint(ScriptPointType type, bool shouldBreak, uint32 index, uint32 offset) {
	assert(type != ScriptPointType::kInvalid);
	for (uint32 i = 0; i < _points.size(); i++) {
		if (_points[i]._type != type)
			continue;
		if (pointTypeNeedsIndex(type) && _points[i]._index != index)
			continue;
		if (pointTypeNeedsOffset(type) && _points[i]._offset != offset)
			continue;
		_points[i]._break = _points[i]._break || shouldBreak;
		return _points[i]._id;
	}
	ScriptTracePoint point;
	point._id = _nextPointId++;
	point._type = type;
	point._index = index;
	point._offset = offset;
	point._break = shouldBreak;
	_points.push_back(point);
	return point._id;
}

bool ScriptDebugger::removePoint(uint32 id) {
	for (uint32 i = 0; i < _points.size(); i++) {
		if (_points[i]._id == id) {
			_points.remove_at(i);
			return true;
		}
	}
	return false;
}

void ScriptDebugger::removeAllPoints() {
	_points.clear();
}

void ScriptDebugger::onCallStart(ScriptCallType type, uint32 index, uint32 offset, uint32 argCount, uint32 lastScopeSize) {
	assert(type != ScriptCallType::kInvalid);
	if (lastScopeSize > 0) {
		if (_callStack.empty())
			warning("Attempted to set scope size at empty call stack");
		else
			_callStack.back()._localScopeSize = lastScopeSize;
	}
	ScriptCallStackEntry entry;
	entry._type = type;
	entry._index = index;
	entry._offset = offset;
	entry._argCount = argCount;
	entry._localScopeSize = argCount;
	entry._localScopeStart = _engine->getScript()->_localScope;
	_callStack.push_back(entry);
	onCallStackModified();
	onCallIncrement(offset);
}

void ScriptDebugger::onCallEnd() {
	assert(!_callStack.empty());
	_callStack.pop_back();
	onCallStackModified();
}

void ScriptDebugger::onCallStackModified() {
	bool shouldBreak = false;
	for (uint32 i = 0; i < _stopsAtCallDepth.size(); i++) {
		if (_stopsAtCallDepth[i] == _callStack.size()) {
			_stopsAtCallDepth.remove_at(i--);
			shouldBreak = true;
		}
	}

	if (shouldBreak)
		breakAndOpenConsole();
}

void ScriptDebugger::onCallIncrement(uint32 newOffset) {
	assert(!_callStack.empty());
	auto &cur = _callStack.back();
	cur._offset = newOffset;

	auto shouldBreak = _stopsNextStep;
	for (auto &point : _points) {
		if (point.appliesTo(cur)) {
			printPointReached(point);
			shouldBreak = shouldBreak || point._break;
		}
	}

	_stopsNextStep = false;
	if (shouldBreak)
		breakAndOpenConsole();
}

void ScriptDebugger::onVariable(bool isWrite, uint32 index) {
	bool shouldBreak = false;
	for (auto &point : _points) {
		if (point._index != index)
			continue;
		if (point._type == ScriptPointType::kVariableAccess ||
			(point._type == ScriptPointType::kVariableRead && !isWrite) ||
			(point._type == ScriptPointType::kVariableWrite && isWrite)) {
			printPointReached(point);
			shouldBreak = shouldBreak || point._break;
		}
	}
	if (shouldBreak)
		breakAndOpenConsole();
}

void ScriptDebugger::onResource(bool isLoad, uint32 index) {
	bool shouldBreak = false;
	for (auto &point : _points) {
		if (point._index != index)
			continue;
		if (point._type == ScriptPointType::kResourceAccess ||
			(point._type == ScriptPointType::kResourceLoad && isLoad)) {
			printPointReached(point);
			shouldBreak = shouldBreak || point._break;
		}
	}
	if (shouldBreak)
		breakAndOpenConsole();
}

void ScriptDebugger::onScene(bool isChanged) {
	bool shouldBreak = false;
	for (auto &point : _points) {
		if ((point._type == ScriptPointType::kSceneChanging && !isChanged) ||
			(point._type == ScriptPointType::kSceneChanged && isChanged)) {
			printPointReached(point);
			shouldBreak = shouldBreak || point._break;
		}
	}
	if (shouldBreak)
		breakAndOpenConsole();
}

void ScriptDebugger::printPointReached(const ScriptTracePoint &point) {
	_engine->getDebugger()->debugPrintf("%s point %d reached: %s %d @ %d\n",
		point._break ? "break" : "trace",
		point._id,
		scriptPointTypeNames[(int)point._type],
		point._index,
		point._offset);
}

void ScriptDebugger::breakAndOpenConsole() {
	if (_isPaused) {
		warning("Debugger is already broken and console is open");
		return;
	}
	_isPaused = true;
	auto debugger = _engine->getDebugger();
	debugger->debugPrintf("EOM\n");
	debugger->attach();
	debugger->onFrame();
	_isPaused = false;
}

void ScriptDebugger::runContinue() {
	if (_isPaused)
		GUI::GuiManager::instance().exitLoop();
	else
		warning("Cannot continue debugger as it is not paused");
}

void ScriptDebugger::runStep() {
	_stopsNextStep = true;
	runContinue();
}

void ScriptDebugger::runStepOver() {
	_stopsAtCallDepth.push_back(_callStack.size());
	runContinue();
}

void ScriptDebugger::runStepOut() {
	if (_callStack.empty()) {
		warning("Cannot step out as call stack is empty");
		return;
	}
	_stopsAtCallDepth.push_back(_callStack.size() - 1);
	runContinue();
}

void ScriptDebugger::printAllPoints() {
	auto debugger = _engine->getDebugger();
	for (const auto &point : _points) {
		debugger->debugPrintf("%3d: %s %s %d @ %d\n",
			point._id,
			point._break ? "break for" : "trace",
			scriptPointTypeNames[(int)point._type],
			point._index,
			point._offset);
	}
}

void ScriptDebugger::printStacktrace(bool onlyFirst) {
	auto debugger = _engine->getDebugger();
	for (uint32 i = 0; i < _callStack.size(); i++) {
		const auto &call = _callStack[_callStack.size() - 1 - i];
		debugger->debugPrintf("%3d: %s %d @ %d",
			i,
			scriptCallTypeNames[(int)call._type],
			call._index,
			call._offset);
		if (call._argCount > 0)
			debugger->debugPrintf(" %d args", call._argCount);
		if (call._localScopeSize > 0)
			debugger->debugPrintf(" %d local variables", call._localScopeSize);
		debugger->debugPrintf("\n");

		if (onlyFirst)
			return;
	}
}

void ScriptDebugger::printLocalScope(uint32 index) {
	auto debugger = _engine->getDebugger();
	if (index == UINT32_MAX) {
		if (_callStack.empty()) {
			debugger->debugPrintf("Call stack empty, there is no local scope\n");
			return;
		}
		index = 0;
	}
	if (index >= _callStack.size()) {
		debugger->debugPrintf("Invalid call index %d, there are only %d calls\n", index, _callStack.size());
		return;
	}
	const auto &call = _callStack[_callStack.size() - 1 - index];
	if (call._type != ScriptCallType::kRoot && call._type != ScriptCallType::kCalc) {
		debugger->debugPrintf("There is no local scope in %s calls\n", scriptCallTypeNames[(int)call._type]);
		return;
	}

	const auto script = _engine->getScript();
	const auto localScopeEnd = index == 0
		? script->_localVariables.size()
		: call._localScopeStart + call._localScopeSize;
	if (localScopeEnd > script->_localVariables.size()) {
		debugger->debugPrintf("Corrupted local scope range\n");
		return;
	}

	for (auto i = call._localScopeStart; i < localScopeEnd; i++)
		debugger->debugPrintf("%3d = %d\n", i - call._localScopeStart, script->_localVariables[i]);
}

void ScriptDebugger::printSceneVariables(uint32 offset, uint32 count) {
	auto debugger = _engine->getDebugger();
	if (_engine->_curSceneIndex >= _engine->_scenes.size()) {
		debugger->debugPrintf("No scene loaded or corrupted scene index\n");
		return;
	}

	const auto scene = _engine->getScene();
	if (offset == UINT32_MAX) {
		offset = 0;
		count = _engine->getGameDesc()->_sceneVarCount;
	}
	const auto end = offset + count;
	if (end > _engine->getGameDesc()->_sceneVarCount) {
		debugger->debugPrintf("Invalid variable range, there are only %d scene variables\n", (int)_engine->getGameDesc()->_sceneVarCount);
		return;
	}

	for (auto i = offset; i < end; i++)
		debugger->debugPrintf("%5d = %d\n", (int)i, scene->getVariable(i));
}

void ScriptDebugger::printSystemVariables(uint32 offset, uint32 count) {
	auto debugger = _engine->getDebugger();
	const auto end = offset + count;
	if (offset == UINT32_MAX) {
		offset = 0;
		count = _engine->getGameDesc()->_systemVarCount;
	}
	if (end > _engine->getGameDesc()->_systemVarCount) {
		debugger->debugPrintf("Innvalid variable range, there are only %d system variables\n", (int)_engine->getGameDesc()->_systemVarCount);
		return;
	}

	const auto script = _engine->getScript();
	for (auto i = offset; i < end; i++)
		debugger->debugPrintf("%5d = %d\n", (int)i, script->_systemVariables[i]);
}

void ScriptDebugger::printDynamicStrings(uint32 offset, uint32 count) {
	auto debugger = _engine->getDebugger();
	if (_engine->_curSceneIndex >= _engine->_scenes.size()) {
		debugger->debugPrintf("No scene loaded or corrupted scene index\n");
		return;
	}

	const auto scene = _engine->getScene();
	const auto totalCount = _engine->getResourceFile()->_dynamicStringCount;
	if (offset == UINT32_MAX) {
		offset = 0;
		count = totalCount;
	}
	const auto end = offset + count;
	if (end > totalCount) {
		debugger->debugPrintf("Invalid variable range, there are only %d global variables\n", (int)totalCount);
		return;
	}

	for (auto i = offset; i < end; i++) {
		const auto value = Common::toPrintable(scene->getDynamicString(i), false);
		debugger->debugPrintf("%5d = %s\n", (int)i, value.c_str());
	}
}

}

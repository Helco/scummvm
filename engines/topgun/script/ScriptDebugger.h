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

#ifndef TOPGUN_SCRIPTDEBUGGER_H
#define TOPGUN_SCRIPTDEBUGGER_H

#include "common/array.h"
#include "gui/debugger.h"

namespace TopGun {

enum class ScriptCallType {
	kRoot,
	kCalc,
	kProcedure,
	kInvalid
};

struct ScriptCallStackEntry {
	ScriptCallType _type;
	uint32 _index;
	uint32 _offset;
	uint32 _localScopeStart;
	uint32 _localScopeSize;
	uint32 _argCount;
};

enum class ScriptPointType {
	kScript,
	kProcedure,
	kVariableRead,
	kVariableWrite,
	kVariableAccess,
	kResourceLoad,
	kResourceAccess,
	kSceneChanging,
	kSceneChanged,
	kInvalid
};

struct ScriptTracePoint {
	uint32 _id;
	ScriptPointType _type;
	uint32 _index;
	uint32 _offset;
	bool _break;

	bool appliesTo(const ScriptCallStackEntry &call);
};

class TopGunEngine;

class ScriptDebugger {
public:
	ScriptDebugger(TopGunEngine *engine);
	~ScriptDebugger() = default;

	uint32 addPoint(ScriptPointType type, bool shouldBreak, uint32 index, uint32 offset = 0);
	bool removePoint(uint32 id);
	void removeAllPoints();

	void onCallStart(ScriptCallType type, uint32 index, uint32 argCount = 0, uint32 lastScopeSize = 0);
	void onCallIncrement(uint32 newOffset);
	void onCallEnd();
	void onVariable(bool isWrite, uint32 index);
	void onResource(bool isLoad, uint32 index);
	void onScene(bool isChanged);

	void runContinue();
	void runStep();
	void runStepOver();
	void runStepOut();

	void printAllPoints();
	void printStacktrace(bool onlyFirst = false);
	void printLocalScope(uint32 index = UINT32_MAX);
	void printGlobalVariables(uint32 offset = UINT32_MAX, uint32 count = 1);
	void printDynamicStrings(uint32 offset = UINT32_MAX, uint32 count = 1);

	static bool pointTypeNeedsIndex(ScriptPointType type);
	static bool pointTypeNeedsOffset(ScriptPointType type);
private:
	void onCallStackModified();
	void printPointReached(const ScriptTracePoint &point);
	void breakAndOpenConsole();

private:
	TopGunEngine *_engine;
	Common::Array<ScriptTracePoint> _points;
	Common::Array<ScriptCallStackEntry> _callStack;
	Common::Array<uint32> _stopsAtCallDepth;

	uint32 _nextPointId;
	bool _stopsNextStep;
	bool _isPaused;
};

}

#endif // TOPGUN_SCRIPTDEBUGGER_H

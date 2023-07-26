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

#ifndef TOPGUN_SCRIPT_H
#define TOPGUN_SCRIPT_H

#include "common/memstream.h"
#include "topgun/script/IPlugin.h"
#include "topgun/script/ScriptDebugger.h"
#include "topgun/Scene.h"

using Common::Array;
using Common::HashMap;

namespace TopGun {

class TopGunEngine;

constexpr int32 kWindowsKeyCount = 256;

/*
 * There are not one, not two, but three different script languages
 * present in TopGun games:
 *   - "Root": A CISC-like language which is always called first (hence the name)
 *             It uses a set of local variables with unknown count. Parameters
 *             are passed (in reverse) as the first n variables.
 *             There is a special register to return values.
 *   - "Calc": A stack-based RISC language called by various "Root"-Instructions.
 *             These are mostly calculation instructions but it is also used for
 *             very simple global variable manipulation.
 *   - "Procedure": There is one "Calc" instruction that can call a function
 *             by a numeric ID with a set of arguments. These are either internally
 *             implemented or loaded from a plugin DLL declared in the resource file.
 *             We emulate the plugin DLLs of course.
 *			   Many IDs are shared with "Root" but the operands are passed in another way.
 *			   Nevertheless until now we can use the same enumeration
 *
 * All of the script languages as well as sprites can use the global variable table
 * stored in the Scene class as well as the system variable table stored here.
 *
 * Of course the script languages are architecture-dependent so we wrap most of the
 * read calls.
 * 
 * There is some hack in a few script instructions where the original game modifies
 * the script e.g. for incrementing a counter.
 */

enum class ScriptOp {
#define TOPGUN_SCRIPT_OP(name) k##name
#include "ScriptOps.h"
#undef TOPGUN_SCRIPT_OP
};

enum class ScriptCalcOp {
	kExit = 0,
	kPushValue,
	kPushVar,
	kReadVarArray,
	kPushVarAddress,
	kReadVar,
	kOffsetVar,
	kWriteVar = 8,
	kCallProc = 13,
	kRunScript,
	kNegate,
	kBooleanNot,
	kBitNot,
	kAdd,
	kSub,
	kMul,
	kDiv,
	kMod,
	kEquals,
	kLessOrEquals,
	kLess,
	kGreaterOrEquals,
	kGreater,
	kNotEquals,
	kBooleanAnd,
	kBooleanOr,
	kBitAnd,
	kBitOr,
	kBitXor,
	kPreIncrementVar,
	kPostIncrementVar,
	kPreDecrementVar,
	kPostDecrementVar,
	kJumpNonZero,
	kJumpZero,
	kPushVarValue,
	kShiftLeft,
	kShiftRight
};

enum class ScriptSystemVariable : int32 {
	kMouseButton = 0,
	kMousePosX = 4,
	kMousePosY,
	kMouseDownPosX,
	kMouseDownPosY,
};

enum class ScriptMouseEvent : int32 {
	kButtonDown = 1,
	kButtonUp = 2,
	kMove = 4
};

struct ScriptKeyListener {
	uint32 _scriptUnmodified,
		_scriptShift,
		_scriptControl,
		_scriptShiftAndControl,
		_scriptUp;
	bool _isDisabled;

	void setDownScript(uint32 script, bool isForShift, bool isForControl);
};

struct ScriptTimer {
	int32 _id;
	uint32 _script;
	uint32 _duration;
	uint32 _nextTrigger;
	bool _repeats;
};

struct QueuedMessage {
	static constexpr int kMaxArguments = 4;
	uint32 _script;
	uint32 _argCount;
	int32 _args[kMaxArguments];
};

class Script {
	friend class ScriptDebugger;
public:
	Script(TopGunEngine *engine);
	~Script();

	void handleEnginePause(bool pause);
	void updateTimers();
	void pauseTimers(bool pause);

	void runMessageQueue();
	void runEntry(); ///< also sets up a new scene (e.g. loads plugin procedures)
	int32 runMessage(uint32 index);
	int32 runMessage(uint32 index, int32 arg);
	int32 runMessage(uint32 index, uint32 localScopeSize, uint32 argCount, const int32 *args);
	void runQueueRootOp(Common::Array<byte> &data, uint32 index);
	int32 runProcedure(uint32 procId, const int32 *args, uint32 argCount, uint32 localScopeSize = 0);
	void postMessage(uint32 index, uint32 argCount, const int32 *args);

	void runKeyDownListener(Common::KeyState keyState);
	void runKeyUpListener(Common::KeyState keyState);
	void setKeyListener(int32 key, uint32 script, bool isForShift, bool isForControl);
	void setKeyUpListener(int32 key, uint32 script);
	void toggleKeyListener(int32 key, bool toggle);
	bool runKeyDownEvent(int32 key);
	bool runMouseEvent(ScriptMouseEvent event);
	void postSpritePicked(uint32 sprite, bool entered);

	int32 evalValue(int32 valueOrIndex, bool isIndex);
	inline int32 evalValue(ValueOrIndirect value) {
		return evalValue(value._value, value._isIndirect);
	}
	void setSystemVariable(ScriptSystemVariable variable, int32 value);
	Common::String getString(int32 index);
	void setString(int32 index, const Common::String &value);
	bool isConstString(int32 index) const;

	inline bool hasSpritePickedHandler() const {
		return _spritePickedEventHandler != 0;
	}

	inline ScriptDebugger *getDebugger() {
		return _debugger.get();
	}

private:
	void runScript(uint32 index);
	void runRoot(Common::MemorySeekableReadWriteStream &stream, uint32 scriptIndex = UINT32_MAX);
	void runSingleRootInstruction(Common::MemorySeekableReadWriteStream &stream, uint32 scriptIndex = UINT32_MAX);
	int32 runCalc(Common::SeekableReadStream &stream, uint32 scriptIndex = UINT32_MAX);
	int32 runInternalProcedure(uint32 procId, const int32 *args, uint32 argCount);
	int32 runPluginProcedure(uint32 procId, const int32 *args, uint32 argCount);

	int32 readSint(Common::ReadStream &stream);
	uint32 readUint(Common::ReadStream &stream);
	void setVariable(int32 index, int32 value);
	void setupLocalArguments(const int32 *args, uint32 argCount);
	int32 stackTop() const;
	int32 stackPop();
	void stackPush(int32 value);
	int32 calcJumpOffset(uint32 nativeIntCount, uint32 additionalBytes = 0) const;
	void jumpToCase(Common::SeekableReadStream &stream,
					int32 switchValue,
					uint32 offsetToCases,
					uint32 caseCount,
					int32 defaultJumpDistance,
					int64 startPos);
	int32 simpleCalc(int32 left, int32 right, byte op, bool negateRight, bool isRightIndirect);
	bool simpleCondition(int32 left, int32 right, byte op);
	void prepareSceneChange();

	void setTimer(int32 id, uint32 script, uint32 duration, bool repeats);
	void deleteTimer(int32 id);
	bool setSpriteQueue(uint32 spriteIndex, uint32 queueIndex, bool hideSprite);

	struct FormatValue {
		bool _isInteger;
		Common::String _string;
		int32 _integer;
	};
	static Common::String sprintfWithArray(const Common::String &format, const Array<FormatValue> &values);

	// A method (also originally present in the engine) used by 10 script ops/procedures
	// that all rely on the same weird internal behaviour of this method.
	struct SetClickRectOp {
		bool _modifyAll = false,
			_doDisable = false,
			_doEnable = false;
		uint32 _spriteIndex = 0,
			_scriptIndex = 0;
		int32 _scriptArg = 0;
		Rect _rect;
	};
	void setClickRect(const SetClickRectOp &op);

private:
	ScopedPtr<ScriptDebugger> _debugger;
	TopGunEngine *_engine;
	Scene *_scene = nullptr;

	int32 _reg3E3F = 0;
	int32 _keyDownEventHandler = 0;
	int32 _mouseEventHandler = 0;
	int32 _pauseEventHandler = 0;
	int32 _spritePickedEventHandler = 0;
	ScriptKeyListener _keyListeners[kWindowsKeyCount];

	bool _areTimersPaused = false, _wereTimersPausedByGameplay = false;
	uint32 _timeAtPausingTimers = 0,
		_curTimerIndex = 0;
	Common::Array<ScriptTimer> _timers;

	int32 _scriptResult = 0;
	uint32 _nestedScriptCount = 0;
	uint32 _localScope = 0; ///< index of first local variable in current scope
	Array<int32> _systemVariables;
	Array<int32> _localVariables;
	Array<int32> _stack; // using Array to easily grab a couple arguments for runProcedure
	Array<ScriptPluginProcedure *> _pluginProcedures;
	Array<QueuedMessage> _messageQueues[2];
	Array<QueuedMessage> *_curMessageQueue;
};

}

#endif // TOPGUN_SCRIPT_H

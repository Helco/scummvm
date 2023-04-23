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
#include "common/hashmap.h"
#include "topgun/Scene.h"

using Common::Array;
using Common::HashMap;

namespace TopGun {

class TopGunEngine;

/*
 * There are not one, not two, but three different script languages
 * present in TopGun games:
 *   - "Root": A CISC language which is always called first (hence the name)
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
 *			   Some procedures are also "Root"-instructions, but in newer games
 *			   these instructions are not used anymore.
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

enum class ScriptRootOp {
	kRunMessage = 1,
	kNop,
	kSetReg = 4,
	kCalcAngle = 11,
	kSpriteBreakLoops = 13,
	kBrowseEvents14,
	kClickRects16 = 16,
	kSpriteSetClipBox = 17,
	kSpriteCombToBackground = 20,
	kSub4_23 = 23,
	kCosine = 26,
	kSetCursor,
	kSetCursorPos,
	kDebugStr = 30,
	kDeleteIniSection = 35,
	kCursorPos43 = 43,
	kText44,
	kChangeScene54 = 54,
	kCloseWindow57 = 57,
	kSetTmpString = 58,
	kBkgTransparent60 = 60,
	kBkgTransparent61,
	kSetCallScriptProcs62,
	kSpriteGetBounds = 63,
	kSpriteGetInfo,
	kGetDate = 66,
	kGetLineIntersect = 72,
	kSpriteGetPos = 77,
	kGetClock = 83,
	kRunScriptIf = 89,
	kJumpIf,
	kJumpIfCalc,
	kRunScriptIfResLoaded = 92,
	kBufferCDC_94 = 94,
	kBufferCDC_96 = 96,
	kBufferCDC_97 = 97,
	kBufferCDC_99 = 99,
	kSetBuffer3E5_101 = 101,
	kJump = 104,
	kSet1943_107 = 107,
	kGetKeyState = 106,
	kDeleteTimer = 109,
	kSpriteSetLevel = 113,
	kSetErrFile = 114,
	kSend4C8_116 = 116,
	kLoadResource = 117,
	kSet3EF7_120 = 120,
	kAudioMute = 122,
	kSpriteOffset = 124,
	kSet3F0B_138 = 138,
	kAudioPlayCDTrack = 142,
	kAudioPlayMidi,
	kAudioPlayWave = 145,
	kPost = 149,
	kRandomValue = 153,
	kRunRandomOf,
	kReadIni = 156,
	kReturn = 161,
	kExit = 162,
	kBackupIni = 163,
	kAnimate = 164,
	kRunNextOf = 167,
	kSimpleCalc = 169,
	kRunCalc = 175,
	kSpriteChangePalette = 179,
	kLoadPaletteResource = 180,
	kExtractFile = 183,
	kSpriteSetPos = 184,
	kSpriteSetQueue = 185,
	kSetString = 192,
	kSetText = 194,
	kSetTextNum195 = 195,
	kSetTimer = 196,
	kAudioSetVolume = 198,
	kSine = 202,
	kAudioStopCD = 210,
	kAudioStopMidi,
	kAudioStopWave,
	kStringCompare,
	kStringCompareI,
	kRunArrayOp = 217,
	kSwitch,
	kCalcSwitch,
	kCase, // not original, is part of Switch/CalcSwitch
	kSpriteSwap222 = 222,
	kFreeResource = 223,
	kMath224 = 224,
	kSpriteIsVisible = 225,
	kJumpIfCalc_alt = 227, // seems to be actually duplicated
	kWriteIni = 228,
	kSetMapTransform = 230
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

class Script {
public:
	Script(TopGunEngine *engine);

	void runEntry();
	void run(uint32 index);
	void runRoot(Common::MemorySeekableReadWriteStream &stream);
	void runSingleRootInstruction(Common::MemorySeekableReadWriteStream &stream);
	int32 runCalc(Common::SeekableReadStream &stream);
	int32 runProcedure(uint32 procId, const int32 *args, uint32 argCount);

private:
	int32 runInternalProcedure(uint32 procId, const int32 *args, uint32 argCount);
	int32 runPluginProcedure(uint32 procId, const int32 *args, uint32 argCount);

	int32 readSint(Common::ReadStream &stream);
	uint32 readUint(Common::ReadStream &stream);
	int32 evalValue(int32 valueOrIndex, bool isIndex);
	void setVariable(int32 index, int32 value);
	void setupLocalArguments(int32 *args, uint32 argCount);
	int32 stackTop() const;
	int32 stackPop();
	void stackPush(int32 value);

private:
	TopGunEngine *_engine;
	Scene *_scene;

	int32 _scriptResult;
	uint32 _nestedScriptCount;
	uint32 _localScope; ///< index of first local variable in current scope
	Array<int32> _systemVariables;
	Array<int32> _localVariables;
	Array<int32> _stack; // using Array to easily grab a couple arguments for runProcedure
};

}

#endif // TOPGUN_SCRIPT_H

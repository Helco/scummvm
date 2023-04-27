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
#include "topgun/Scene.h"

using Common::Array;
using Common::HashMap;

namespace TopGun {

class TopGunEngine;

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
	// operations I am not sure about have their index as suffix

	kRunMessage = 1,
	kNop = 2,
	kAbsolute = 3,
	kSetReg3E43 = 4,
	kSetScriptReg3E3F = 5,
	kFixedPointAdd = 6,
	kSetAudioVolume7 = 7,
	kSetAudioVolume8 = 8,
	kSetAudioVolume9 = 9,
	kSetAudioVolume10 = 10,
	kCalcAngle = 11,
	kStringToInt = 12,
	kSpriteBreakLoops = 13,
	kBrowseEvents14,
	kSetOnSpritePicked = 15,
	kClickRects16 = 16,
	kSpriteSetClipBox17 = 17,
	kCollide18 = 18,
	kSpriteCombToBackground = 20,
	kBitmapMerge21 = 21,
	kBitmapMerge22 = 22,
	kSetHitDetectTrigger23 = 23,
	kAreSpriteHitting = 24,
	kCopyResource = 25,
	kCosine = 26,
	kSetCursor = 27,
	kSetCursorPos28 = 28,
	kSpriteCurvePoint = 29,
	kDebugStr = 30,
	kBrowseEvents31 = 31,
	kBrowseEvents32 = 32,
	kSetClickRect33 = 33,
	kFreeResource = 34,
	kDeleteIniSection = 35,
	kSetClickRect36 = 36,
	kArray1943_37 = 37,
	kArray1943_38 = 38,
	kSetClickRect39 = 39,
	kSetHitDetectTrigger40 = 40,
	kDistance = 41,
	kFixedPointDiv = 42,
	kPickedSprite43 = 43,
	kStartTextInput = 44,
	kSetQueue45 = 45,
	kSetClickRect46 = 46,
	kSetClickRect47 = 47,
	kBufferCDC_48 = 48,
	kArray1943_50 = 50,
	kArray1943_51 = 51,
	kSetClickRect52 = 52,
	kPickedSprite53 = 53,
	kChangeScene54 = 54,
	kScene55 = 55,
	kScene56 = 56,
	kScene57 = 57,
	kSetTmpString = 58,
	kGetWinIconOrBitmap = 59,
	kFade = 60,
	kStopFade = 61,
	kSetCallScriptProcs62 = 62,
	kSpriteGetBounds63 = 63,
	kSpriteGetInfo64 = 64,
	kGetStringChar = 65,
	kGetDate = 66,
	kGetDate_dup = 67, // duplicated operation
	kGetTableValue68 = 68,
	kGetTableValue69 = 69,
	kGetDriveType = 70,
	kGetFreeGlobalMemory = 71,
	kGetLineIntersect = 72,
	kGetScriptReg3EF7_73 = 73,
	kSpriteGetNumCells = 74,
	kGetTableHeaderValue75 = 75,
	kGetWindowsVersion = 76,
	kSpriteGetPos = 77,
	kGetQuadrant = 78,
	kSpriteGetScrollBox = 79,
	kSpriteGetScrollPos = 80,
	kSpriteGetInfo81 = 81,
	kGetTableValue82 = 82,
	kGetClock = 83,
	kAddAtom85 = 85,
	kGetUserName = 86,
	kAudioGetWaveSoundTime = 87,
	kRunScriptIf = 89,
	kJumpIf = 90,
	kJumpIfCalc = 91,
	kRunScriptIfResLoaded = 92,
	kRunArrayOp93 = 93,
	kBufferCDC_94 = 94,
	kSetKeyBind95 = 95,
	kBufferCDC_96 = 96,
	kBufferCDC_97 = 97,
	kbufferCDC_98 = 98,
	kBufferCDC_99 = 99,
	kIsResourceLoaded = 100,
	kSetBuffer3E5_101 = 101,
	kSetToggle95C3 = 102,
	kSetToggle95C4 = 103,
	kJump = 104,
	kSetReg3EE7_105 = 105,
	kGetKeyState = 106,
	kSetReg3EE7_107 = 107,
	kDeleteTimer = 109,
	kDeleteTimer_dup = 110, // duplicated operation
	kWinExec = 111,
	kSpriteSetLevel = 113,
	kSetErrFile = 114,
	kSpriteLinePoint = 115,
	kScene116 = 116,
	kLoadResource = 117,
	kMax = 118,
	kMin = 119,
	kSet3EF7_120 = 120,
	kFixedPointMul = 121,
	kAudioMute = 122,
	kAudioMute_dup = 123,
	kSpriteOffset = 124,
	kSetClickRect125 = 125,
	kDeleteClickRect = 126,
	kSpriteSetData127 = 127,
	kbufferCDC_128 = 128,
	karray1943_129 = 129,
	karray1943_130 = 130,
	kSetReg3EF7 = 131,
	kBrowseEvents132 = 132,
	kSetClickRect133 = 133,
	kSetClickRect134 = 134,
	kSetHitDetectTrigger135 = 135,
	kPickedSprite136 = 136,
	kAudioPause = 137,
	kSet3F0B_138 = 138,
	kAudioPauseMidi = 139,
	kSpritePaused = 140,
	kPauseTimers = 141,
	kAudioPlayCDTrack = 142,
	kAudioPlayMidi = 143,
	kAudioPlayMidi_dup = 144,
	kAudioPlayWave145 = 145,
	kAudioPlayWave146 = 146,
	kAudioPlayWave147 = 147,
	kSetOnPlayWave = 148,
	kPost = 149,
	kPost_dup = 150,
	kPrintBitmap = 151,
	kSpriteIsPicked = 152,
	kRandomValue = 153,
	kRunRandomOf = 154,
	kRunRandomOf_dup = 155,
	kReadIni = 156,
	kGetRegistryString = 157,
	kReflectAngle = 158,
	kSpriteChangePalette160 = 160,
	kReturn = 161,
	kExit = 162,
	kBackupIni = 163,
	kAnimate = 164,
	kRunScriptInSeconds = 165,
	kSpriteScroll = 166,
	kRunNextOf = 167,
	kRunNextOf_dup = 168,
	kSimpleCalc = 169,
	kAnimate170 = 170,
	kAnimate171 = 171,
	kSetStringChar = 172,
	kSpriteSetClipBox173 = 173,
	kBackupCursorPos174 = 174,
	kRunCalc = 175,
	kBufferCDC_176 = 176,
	kMovie178 = 178,
	kSpriteChangePalette179 = 179,
	kLoadPaletteResource = 180,
	kAnimate181 = 181,
	kAnimate182 = 182,
	kExtractFile = 183,
	kSpriteSetPos = 184,
	kSetQueue185 = 185,
	kSetQueue186 = 186,
	kAnimate187 = 187,
	kAnimate188 = 188,
	kSetScrollBox = 189,
	kSpriteSetScrollPos = 190,
	kSpriteSetSpriteClipBox = 191,
	kSetString = 192,
	kGetTableValue193 = 193,
	kSetText = 194,
	kSetTextNum195 = 195,
	kSetTimer = 196,
	kSetTimer_dup = 197,
	kAudioSetVolume = 198,
	kExtractFileOrWallpaper = 199,
	kSpriteScreenShake = 200,
	kSine = 202,
	kSpriteCompToBackground = 203,
	kSpriteGetExtraId = 204,
	kSpritePostMessage = 205,
	kSpriteSendMessage = 206,
	kSpriteSetExtraId = 207,
	kSpriteTransfer = 208,
	kSquareRoot = 209,
	kAudioStopCD = 210,
	kAudioStopMidi = 211,
	kAudioStopWave = 212,
	kStringCompare = 213,
	kStringCompareI = 214,
	kStringLength = 215,
	kStringFind = 216,
	kRunArrayOp217 = 217,
	kSwitch = 218,
	kCalcSwitch = 219,
	kCase = 220, // originally not executed at all, it is part of Switch/CalcSwitch
	kAudioMute221 = 221,
	kSpriteSwap222 = 222,
	kFreeResource223 = 223,
	kMath224 = 224,
	kSpriteIsVisible = 225,
	kSetReg95DC = 226,
	kJumpIfCalc_dup = 227,
	kWriteIni = 228,
	kSetOrDeleteRegistryString = 229,
	kSetMapTransform = 230,
	kSpriteSetSync = 231,
	kBackupAdditionalHMMIO = 232,
	kSetReg_3E4F = 233,
	kUnloadAllOtherScenes = 234,
	kSetScreenRes = 235,
	kSetPauseEventScript = 236,
	kStringConcat = 237,
	kStringCopy = 238,
	kStringCopySized = 239,
	kGetFullScenePath = 240,
	kSpriteGetBounds241 = 241,
	kOpenAdditionHMMIO = 242,
	kSetCursorPos243 = 243,
	kGetRegistryNumber = 244,
	kGetRegistryString_dup = 245,
	kSetOrDeleteRegistryNumber = 246,
	kSetOrDeleteRegistryString_dup = 247,
	kGetRegistryNumberWithSubKey = 248,
	kGetRegistryStringWithSubKey = 249,
	kSetOrDeleteRegistryNumberWithSubKey = 250,
	kSetOrDeleteRegistryStringWithSubKey = 251,
	kGetIniNumber = 252,
	kSetIniNumber = 253,
	kGetIniString = 254,
	kSetIniString = 255,
	kAudioSetWaveSoundPriority = 256,
	kDeleteIniKey = 257,
	kDeleteIniTopic = 258,
	kFileExist = 259,
	kIsSingleGameOpen = 260,
	kSpriteCompToSprite = 261,
	kAssert = 262,
	kMessageBox = 263,
	kGetResolution = 264,
	kSpriteSetSync265 = 265,
	kSpriteAnimate266 = 266,
	kSpriteSetTopMost = 267,
	kGetSecondsSinceB84 = 268,
	kSetSecondsSinceB84 = 269,
	kBitmapMerge270 = 270,
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
	~Script();

	void runEntry(); ///< also sets up a new scene (e.g. loads plugin procedures)
	void run(uint32 index);
	void runRoot(Common::MemorySeekableReadWriteStream &stream);
	void runSingleRootInstruction(Common::MemorySeekableReadWriteStream &stream);
	int32 runCalc(Common::SeekableReadStream &stream);
	int32 runProcedure(uint32 procId, const int32 *args, uint32 argCount);

	int32 evalValue(int32 valueOrIndex, bool isIndex);
	Common::String getString(int32 index);

private:
	int32 runInternalProcedure(uint32 procId, const int32 *args, uint32 argCount);
	int32 runPluginProcedure(uint32 procId, const int32 *args, uint32 argCount);

	int32 readSint(Common::ReadStream &stream);
	uint32 readUint(Common::ReadStream &stream);
	void setVariable(int32 index, int32 value);
	void setupLocalArguments(int32 *args, uint32 argCount);
	int32 stackTop() const;
	int32 stackPop();
	void stackPush(int32 value);
	void setString(int32 index, const Common::String &value);
	int32 calcJumpOffset(uint32 nativeIntCount, uint32 additionalBytes = 0) const;

	void jumpToCase(Common::SeekableReadStream &stream,
					int32 switchValue,
					uint32 offsetToCases,
					uint32 caseCount,
					int32 defaultJumpDistance,
					int64 startPos);
	int32 simpleCalc(int32 left, int32 right, byte op, bool negateRight, bool isRightIndirect);

	struct FormatValue {
		bool _isInteger;
		Common::String _string;
		int32 _integer;
	};
	static Common::String sprintfWithArray(const Common::String &format, const Array<FormatValue> &values);

private:
	TopGunEngine *_engine;
	Scene *_scene;

	int32 _reg3E3F;
	int32 _pauseEventHandler;

	int32 _scriptResult;
	uint32 _nestedScriptCount;
	uint32 _localScope; ///< index of first local variable in current scope
	Array<int32> _systemVariables;
	Array<int32> _localVariables;
	Array<int32> _stack; // using Array to easily grab a couple arguments for runProcedure
	Array<ScriptPluginProcedure *> _pluginProcedures;
};

}

#endif // TOPGUN_SCRIPT_H

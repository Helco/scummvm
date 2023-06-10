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

#include "gui/debugger.h"
#include "topgun/topgun.h"

namespace TopGun {

static void checkArgCount(uint32 actual, uint32 expected) {
	if (actual != expected)
		error("Invalid number of procedure arguments, expected %d but got %d", expected, actual);
}

static void checkArgCount(uint32 actual, uint32 min, uint32 max) {
	if (actual < min || actual > max)
		error("Invalid number of procedure arguments, expected %d-%d but got %d", min, max, actual);
}

const char *internalProcedureNames[] = {
	"Unknown0",
	"RunMessage",
	"Nop",
	"Absolute",
	"SetReg3E43",
	"SetScriptReg3E3F",
	"FixedPointAdd",
	"SetAudioVolume7",
	"SetAudioVolume8",
	"SetAudioVolume9",
	"SetAudioVolume10",
	"CalcAngle",
	"StringToInt",
	"SpriteBreakLoops",
	"BrowseEvents14",
	"SetOnSpritePicked",
	"ClickRects16",
	"SpriteSetClipBox17",
	"Collide18",
	"Unknown19",
	"SpriteCombToBackground",
	"BitmapMerge21",
	"BitmapMerge22",
	"SetHitDetectTrigger23",
	"AreSpriteHitting",
	"CopyResource",
	"Cosine",
	"SetCursor",
	"SetCursorPos28",
	"SpriteCurvePoint",
	"DebugStr",
	"BrowseEvents31",
	"BrowseEvents32",
	"SetClickRect33",
	"FreeResource",
	"DeleteIniSection",
	"SetClickRect36",
	"DeleteKeyListener",
	"DeleteModifiedKeyListener",
	"SetClickRect39",
	"SetHitDetectTrigger40",
	"Distance",
	"FixedPointDiv",
	"PickedSprite43",
	"StartTextInput",
	"SetQueue45",
	"SetClickRect46",
	"SetClickRect47",
	"BufferCDC_48",
	"Unknown49",
	"ToggleKeyListener",
	"ToggleModifiedKeyListener",
	"SetClickRect52",
	"PickedSprite53",
	"ChangeScene54",
	"ChangeScene",
	"QuitScene",
	"ChangeSceneToTmpString",
	"SetTmpString",
	"GetWinIconOrBitmap",
	"Fade",
	"StopFade",
	"SetCallScriptProcs62",
	"SpriteGetBounds63",
	"SpriteGetInfo64",
	"GetStringChar",
	"GetDate",
	"GetDate_dup",
	"GetTableValue68",
	"GetTableValue69",
	"GetDriveType",
	"GetFreeGlobalMemory",
	"GetLineIntersect",
	"GetScriptReg3EF7_73",
	"SpriteGetNumCells",
	"GetTableHeaderValue75",
	"GetWindowsVersion",
	"SpriteGetPos",
	"GetQuadrant",
	"SpriteGetScrollBox",
	"SpriteGetScrollPos",
	"SpriteGetInfo81",
	"GetTableValue82",
	"GetClock",
	"Unknown84",
	"AddAtom85",
	"GetUserName",
	"AudioGetWaveSoundTime",
	"Unknown88",
	"RunScriptIf",
	"JumpIf",
	"JumpIfCalc",
	"RunScriptIfResLoaded",
	"RunArrayOp93",
	"BufferCDC_94",
	"SetKeyBind95",
	"BufferCDC_96",
	"BufferCDC_97",
	"bufferCDC_98",
	"BufferCDC_99",
	"IsResourceLoaded",
	"SetBuffer3E5_101",
	"SetToggle95C3",
	"SetToggle95C4",
	"Jump",
	"SetReg3EE7_105",
	"GetKeyState",
	"SetReg3EE7_107",
	"Unknown108",
	"DeleteTimer",
	"DeleteTimer_dup",
	"WinExec",
	"Unknown112",
	"SpriteSetLevel",
	"SetErrFile",
	"SpriteLinePoint",
	"ClearTopMostSpriteNextFrame",
	"LoadResource",
	"Max",
	"Min",
	"Set3EF7_120",
	"FixedPointMul",
	"AudioMute",
	"AudioMute_dup",
	"SpriteOffset",
	"SetClickRect125",
	"DeleteClickRect",
	"SpriteSetData127",
	"bufferCDC_128",
	"SetKeyListener",
	"SetModifiedKeyListener",
	"SetMouseEventListener",
	"BrowseEvents132",
	"SetClickRect133",
	"SetClickRect134",
	"SetHitDetectTrigger135",
	"PickedSprite136",
	"AudioPause",
	"Set3F0B_138",
	"AudioPauseMidi",
	"SpritePaused",
	"PauseTimers",
	"AudioPlayCDTrack",
	"AudioPlayMidi",
	"AudioPlayMidi_dup",
	"AudioPlayWave145",
	"AudioPlayWave146",
	"AudioPlayWave147",
	"SetOnPlayWave",
	"Post",
	"Post_dup",
	"PrintBitmap",
	"SpriteIsPicked",
	"RandomValue",
	"RunRandomOf",
	"RunRandomOf_dup",
	"ReadIni",
	"GetRegistryString",
	"ReflectAngle",
	"Unknown159",
	"SpriteChangePalette160",
	"Return",
	"Exit",
	"BackupIni",
	"Animate",
	"RunScriptInSeconds",
	"SpriteScroll",
	"RunNextOf",
	"RunNextOf_dup",
	"SimpleCalc",
	"SetBackgroundBitmap",
	"SetBackgroundBitmapWithAnimation",
	"SetStringChar",
	"SpriteSetClipBox",
	"BackupCursorPos174",
	"RunCalc",
	"BufferCDC_176",
	"Unknown177",
	"Movie178",
	"SpriteChangePalette179",
	"LoadPaletteResource",
	"SetBackgroundColor",
	"SetBackgroundColorWithAnimation",
	"ExtractFile",
	"SpriteSetPos",
	"SetQueue185",
	"SetQueue186",
	"SetBackgroundColorRGB",
	"SetBackgroundColorRGBWithAnimation",
	"SetScrollBox",
	"SpriteSetScrollPos",
	"SpriteSetSpriteClipBox",
	"SetString",
	"GetTableValue193",
	"SetText",
	"SetTextNum195",
	"SetTimer",
	"SetTimer_dup",
	"AudioSetVolume",
	"ExtractFileOrWallpaper",
	"SpriteScreenShake",
	"Unknown201",
	"Sine",
	"SpriteCompToBackground",
	"SpriteGetExtraId",
	"SpritePostMessage",
	"SpriteSendMessage",
	"SpriteSetExtraId",
	"SpriteTransfer",
	"SquareRoot",
	"AudioStopCD",
	"AudioStopMidi",
	"AudioStopWave",
	"StringCompare",
	"StringCompareI",
	"StringLength",
	"StringFind",
	"RunArrayOp217",
	"Switch",
	"CalcSwitch",
	"Case",
	"AudioMute221",
	"SpriteSwap222",
	"FreeResource223",
	"Math224",
	"SpriteIsVisible",
	"SetReg95DC",
	"JumpIfCalc_dup",
	"WriteIni",
	"SetOrDeleteRegistryString",
	"SetMapTransform",
	"SpriteSetSync",
	"BackupAdditionalHMMIO",
	"SetReg_3E4F",
	"UnloadAllOtherScenes",
	"SetScreenRes",
	"SetPauseEventScript",
	"StringConcat",
	"StringCopy",
	"StringCopySized",
	"GetFullScenePath",
	"SpriteGetBounds241",
	"OpenAdditionHMMIO",
	"SetCursorPos243",
	"GetRegistryNumber",
	"GetRegistryString_dup",
	"SetOrDeleteRegistryNumber",
	"SetOrDeleteRegistryString_dup",
	"GetRegistryNumberWithSubKey",
	"GetRegistryStringWithSubKey",
	"SetOrDeleteRegistryNumberWithSubKey",
	"SetOrDeleteRegistryStringWithSubKey",
	"GetIniNumber",
	"SetIniNumber",
	"GetIniString",
	"SetIniString",
	"AudioSetWaveSoundPriority",
	"DeleteIniKey",
	"DeleteIniTopic",
	"FileExist",
	"IsSingleGameOpen",
	"SpriteCompToSprite",
	"Assert",
	"MessageBox",
	"GetResolution",
	"SpriteSetSync265",
	"SpriteAnimate266",
	"SpriteSetTopMost",
	"GetSecondsSinceB84",
	"SetSecondsSinceB84",
	"BitmapMerge270"
};


int32 Script::runInternalProcedure(uint32 procId, const int32 *args, uint32 argCount) {
	if (debugChannelSet(kVerbose, kDebugScript)) {
		debugCN(kVerbose, kDebugScript, "procedure %d %s", procId,
			procId >= sizeof(internalProcedureNames) / sizeof(const char *) ? "<unknown>" : internalProcedureNames[procId]);
		if (argCount > 0) {
			debugCN(kVerbose, kDebugScript, " with");
			for (uint32 i = 0; i < argCount; i++)
				debugCN(kVerbose, kDebugScript, " %d", args[i]);
		}
		debugCN(kVerbose, kDebugScript, "\n");
	}

	switch ((ScriptOp)procId) {
	case ScriptOp::kPost:
	case ScriptOp::kPost_dup:
		checkArgCount(argCount, 3);
		postMessage(args[0], 2, args + 1);
		break;
	case ScriptOp::kSetScriptReg3E3F:
		checkArgCount(argCount, 1);
		_reg3E3F = args[0];
		break;
	case ScriptOp::kSetOnSpritePicked: {
		checkArgCount(argCount, 1);
		auto prevHandler = _spritePickedEventHandler;
		if (!args[0] || _engine->getResourceType(args[0]) == ResourceType::kScript) {
			_engine->leavePickedSprite();
			_spritePickedEventHandler = args[0];
			if (args[0])
				_engine->updatePickedSprite();
		}
		return prevHandler;
	}break;
	case ScriptOp::kSetReg3EE7_105:
	case ScriptOp::kSetReg3EE7_107:
		checkArgCount(argCount, 1);
		_reg3EE7 = args[0];
		break;
	case ScriptOp::kSetCursor:
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setCursor(args[0]);
		break;
	case ScriptOp::kChangeScene:
		checkArgCount(argCount, 2);
		if (args[1])
			_engine->setTopMostSprite(nullptr);
		prepareSceneChange();
		_engine->postChangeScene(getString(args[0]));
		break;
	case ScriptOp::kQuitScene:
		checkArgCount(argCount, 1);
		if (args[0])
			_engine->setTopMostSprite(nullptr);
		prepareSceneChange();
		_engine->postQuitScene();
		break;
	case ScriptOp::kChangeSceneToTmpString:
		checkArgCount(argCount, 0);
		warning("stub procedure kChangeSceneToTmpString");
		debugCN(kInfo, kDebugScript, "Quit game due to empty tmp string in changeSceneToTmpString procedure\n");
		g_engine->quitGame();
		break;
	case ScriptOp::kFade:
		checkArgCount(argCount, 1);
		warning("stub procedure fade");
		break;
	case ScriptOp::kStopFade:
		// TODO: Implement, was postponed because non-essential
		warning("stub procedure kStopFade");
		break;
	case ScriptOp::kGetFreeGlobalMemory:
		// seems to be used for compatibility checks so any number higher is alright
		return INT32_MAX;
	case ScriptOp::kIsResourceLoaded:
		checkArgCount(argCount, 1);
		return _engine->isResourceLoaded(args[0]);
	case ScriptOp::kSpriteSetLevel:
		checkArgCount(argCount, 2);
		if (_engine->isResourceLoaded(args[0]) && _engine->getResourceType(args[0]) == ResourceType::kSprite)
			_engine->loadResource<Sprite>(args[0])->setLevel(args[1]);
		break;
	case ScriptOp::kClearTopMostSpriteNextFrame:
		checkArgCount(argCount, 1);
		_engine->postClearTopMostSprite(args[0]);
		break;
	case ScriptOp::kSpriteTransfer:
		checkArgCount(argCount, 4);
		_engine->getSpriteCtx()->copySpriteTo(args[0], args[1], args[2], args[3]);
		break;
	case ScriptOp::kEmptyQueue:
		checkArgCount(argCount, 2);
		return setSpriteQueue(args[0], 0, args[1]);
	case ScriptOp::kSetQueue:
		checkArgCount(argCount, 2);
		return setSpriteQueue(args[0], args[1], false);
	case ScriptOp::kSetQueueAndHide:
		checkArgCount(argCount, 2, 3);
		return setSpriteQueue(args[0], args[1], argCount < 3 ? false : args[2]);
	case ScriptOp::kSpritePostMessage:
		checkArgCount(argCount, 2, UINT32_MAX);
		_engine->loadResource<Sprite>(args[0])->postMessage(args + 1, argCount - 1);
		break;
	case ScriptOp::kSpriteSendMessage:
		checkArgCount(argCount, 2, UINT32_MAX);
		_engine->loadResource<Sprite>(args[0])->sendMessage(args + 1, argCount - 1);
		break;
	case ScriptOp::kLoadResource:
		checkArgCount(argCount, 1);
		_engine->loadResource(args[0], ResourceType::kInvalid);
		return 1;
	case ScriptOp::kSetPauseEventScript:
		checkArgCount(argCount, 1);
		_pauseEventHandler = args[0];
		break;
	case ScriptOp::kSetTimer:
	case ScriptOp::kSetTimer_dup:
		checkArgCount(argCount, 4);
		setTimer(args[0], args[2], args[1], args[3]);
		break;
	case ScriptOp::kDeleteTimer:
	case ScriptOp::kDeleteTimer_dup:
		checkArgCount(argCount, 1);
		if (args[0] == -1)
			_timers.clear();
		else
			deleteTimer(args[0]);
		break;
	case ScriptOp::kPauseTimers:
		pauseTimers(args[0]);
		break;
		
	case ScriptOp::kSetBackgroundColor:
	case ScriptOp::kSetBackgroundColorWithAnimation:
		checkArgCount(argCount, 1, 4);
		_engine->getSpriteCtx()->setBackground(args[0]);
		break;
	case ScriptOp::kSetBackgroundColorRGB:
	case ScriptOp::kSetBackgroundColorRGBWithAnimation:
		// animation is only supported for bitmap in the original game
		checkArgCount(argCount, 3, 6);
		_engine->getSpriteCtx()->setBackground(args[0], args[1], args[2]);
		break;
	case ScriptOp::kSetBackgroundBitmap:
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setBackground(args[0], args[0]);
		break;
	case ScriptOp::kSetBackgroundBitmapWithAnimation:
		checkArgCount(argCount, 4);
		_engine->getSpriteCtx()->setBackground(args[0], args[0], (BackgroundAnimation)args[1], args[2], args[3]);
		break;
	case ScriptOp::kSpriteSetClipBox:
		checkArgCount(argCount, 4);
		_engine->getSpriteCtx()->setClipBox(Rect(args[0], args[1], args[2], args[3]));
		break;

	case ScriptOp::kSetKeyListener:
		checkArgCount(argCount, 2);
		setKeyListener(args[0], args[1], false, false);
		break;
	case ScriptOp::kSetModifiedKeyListener:
		checkArgCount(argCount, 4);
		setKeyListener(args[0], args[3], args[2] != 0, args[1] != 0);
		break;
	case ScriptOp::kDeleteKeyListener:
		checkArgCount(argCount, 1);
		setKeyListener(args[0], 0, false, false);
		break;
	case ScriptOp::kDeleteModifiedKeyListener:
		checkArgCount(argCount, 3);
		setKeyListener(args[0], 0, args[2] != 0, args[1] != 0);
		break;
	case ScriptOp::kToggleKeyListener:
	case ScriptOp::kToggleModifiedKeyListener:
		checkArgCount(argCount, 2, 4);
		toggleKeyListener(args[0], args[argCount - 1] != 0);
		break;
	case ScriptOp::kSetMouseEventListener: {
		checkArgCount(argCount, 1);
		const auto prevHandler = _mouseEventHandler;
		_mouseEventHandler = args[0];
		return prevHandler;
	}break;

	case ScriptOp::kSetClickRect: {
		checkArgCount(argCount, 6);
		SetClickRectOp op;
		op._rect = Rect(args[0], args[1], args[2], args[3]);
		op._scriptIndex = (uint32)args[4];
		op._scriptArg = args[5];
		setClickRect(op);
	}break;
	case ScriptOp::kSetSpriteClick: {
		checkArgCount(argCount, 3);
		SetClickRectOp op;
		op._spriteIndex = (uint32)args[0];
		op._scriptIndex = (uint32)args[1];
		op._scriptArg = args[2];
		setClickRect(op);
	}break;
	case ScriptOp::kToggleAllClickRects: {
		checkArgCount(argCount, 1);
		SetClickRectOp op;
		op._modifyAll = true;
		op._doDisable = args[0];
		op._doEnable = !args[0];
		setClickRect(op);
	}break;
	case ScriptOp::kToggleClickRect: {
		checkArgCount(argCount, 5);
		SetClickRectOp op;
		op._rect = Rect(args[0], args[1], args[2], args[3]);
		op._doDisable = args[4];
		op._doEnable = !args[4];
		setClickRect(op);
	}break;
	case ScriptOp::kRemoveClickRect: {
		checkArgCount(argCount, 4);
		SetClickRectOp op;
		op._rect = Rect(args[0], args[1], args[2], args[3]);
		setClickRect(op);
	}break;
	case ScriptOp::kClearClickRects: {
		checkArgCount(argCount, 0);
		SetClickRectOp op;
		op._modifyAll = true;
		setClickRect(op);
	}break;
	case ScriptOp::kRemoveSpriteClick: {
		checkArgCount(argCount, 1);
		SetClickRectOp op;
		op._spriteIndex = (uint32)args[0];
		setClickRect(op);
	}break;
	case ScriptOp::kSetAllClickScripts: {
		checkArgCount(argCount, 1);
		SetClickRectOp op;
		op._modifyAll = true;
		op._scriptIndex = (uint32)args[0];
		setClickRect(op);
	}break;
	case ScriptOp::kSetClickRectScripts: {
		checkArgCount(argCount, 1);
		_engine->setClickRectScripts((uint32)args[0]);
	}break;
	case ScriptOp::kSetSpriteClicks: {
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setAllSpriteClickScripts((uint32)args[0]);
	}break;
	case ScriptOp::kSetSpriteClickable: {
		checkArgCount(argCount, 2);
		SetClickRectOp op;
		op._spriteIndex = (uint32)args[0];
		op._doDisable = args[1];
		op._doEnable = !args[1];
		setClickRect(op);
	}break;

	case ScriptOp::kGetRegistryString:
	case ScriptOp::kGetRegistryString_dup: {
		checkArgCount(argCount, 3, 4);
		const auto newValue = _engine->getSavestate()->getRegistryString(
			Savestate::kRegistryLocalMachineKey,
			nullptr,
			getString(args[0]).c_str(),
			getString(args[1]).c_str());
		setString(args[2], newValue);
	}break;
	case ScriptOp::kSetOrDeleteRegistryString:
	case ScriptOp::kSetOrDeleteRegistryString_dup: {
		checkArgCount(argCount, 3, 4);
		const auto newValue = getString(args[2]);
		if (newValue.size())
			_engine->getSavestate()->setRegistryString(
				Savestate::kRegistryLocalMachineKey,
				nullptr,
				getString(args[0]).c_str(),
				getString(args[1]).c_str(),
				newValue.c_str());
		else
			_engine->getSavestate()->deleteRegistryValue(
				Savestate::kRegistryLocalMachineKey,
				nullptr,
				getString(args[0]).c_str(),
				getString(args[1]).c_str());
	}break;
	case ScriptOp::kGetRegistryNumber: {
		checkArgCount(argCount, 3, 4);
		const auto newValue = _engine->getSavestate()->getRegistryNumber(
			Savestate::kRegistryLocalMachineKey,
			nullptr,
			getString(args[0]).c_str(),
			getString(args[1]).c_str());
		setVariable(args[2], newValue);
	}break;
	case ScriptOp::kSetOrDeleteRegistryNumber: {
		checkArgCount(argCount, 3, 5);
		if (argCount < 4 || args[3] == 0)
			_engine->getSavestate()->setRegistryNumber(
				Savestate::kRegistryLocalMachineKey,
				nullptr,
				getString(args[0]).c_str(),
				getString(args[1]).c_str(),
				args[2]);
		else
			_engine->getSavestate()->deleteRegistryValue(
				Savestate::kRegistryLocalMachineKey,
				nullptr,
				getString(args[0]).c_str(),
				getString(args[1]).c_str());
	}break;
	case ScriptOp::kGetRegistryNumberWithSubKey: {
		checkArgCount(argCount, 5, 6);
		const auto newValue = _engine->getSavestate()->getRegistryNumber(
			args[0],
			getString(args[1]).c_str(),
			getString(args[2]).c_str(),
			getString(args[3]).c_str());
		setVariable(args[4], newValue);
	}break;
	case ScriptOp::kGetRegistryStringWithSubKey: {
		checkArgCount(argCount, 5, 6);
		const auto newValue = _engine->getSavestate()->getRegistryString(
			args[0],
			getString(args[1]).c_str(),
			getString(args[2]).c_str(),
			getString(args[3]).c_str());
		setString(args[4], newValue);
	}break;
	case ScriptOp::kSetOrDeleteRegistryNumberWithSubKey: {
		checkArgCount(argCount, 5, 7);
		if (argCount < 6 || args[5] == 0)
			_engine->getSavestate()->setRegistryNumber(
				args[0],
				getString(args[1]).c_str(),
				getString(args[2]).c_str(),
				getString(args[3]).c_str(),
				args[4]);
		else
			_engine->getSavestate()->deleteRegistryValue(
				args[0],
				getString(args[1]).c_str(),
				getString(args[2]).c_str(),
				getString(args[3]).c_str());
	}break;
	case ScriptOp::kSetOrDeleteRegistryStringWithSubKey: {
		checkArgCount(argCount, 5, 6);
		const auto newValue = getString(args[5]);
		if (newValue.size())
			_engine->getSavestate()->setRegistryString(
				args[0],
				getString(args[1]).c_str(),
				getString(args[2]).c_str(),
				getString(args[3]).c_str(),
				newValue.c_str());
		else
			_engine->getSavestate()->deleteRegistryValue(
				args[0],
				getString(args[1]).c_str(),
				getString(args[2]).c_str(),
				getString(args[3]).c_str());
	}break;

	case ScriptOp::kAudioPlayWave146:
		warning("stub procedure AudioPlayWave146");
		break;
	default:
		if (procId >= sizeof(internalProcedureNames) / sizeof(const char *))
			error("Unknown internal procedure: %d", procId);
		else
			error("Unimplemented internal procedure: %s (%d)", internalProcedureNames[procId], procId);
	}

	return static_cast<int32>(procId);
}

bool Script::setSpriteQueue(uint32 spriteIndex, uint32 queueIndex, bool hideSprite) {
	if (!spriteIndex || _engine->getResourceType(spriteIndex) != ResourceType::kSprite)
		return false;
	if (!queueIndex && !_engine->isResourceLoaded(spriteIndex))
		return true;

	auto sprite = _engine->loadResource<Sprite>(spriteIndex);
	if (hideSprite)
		sprite->setVisible(false);
	if (queueIndex) {
		if (_engine->getResourceType(queueIndex) == ResourceType::kQueue)
			sprite->setQueue(_engine->loadResource<SpriteMessageQueue>(queueIndex).get());
		else
			return false;
	}
	else
		sprite->clearQueue();
	return true;
}

}

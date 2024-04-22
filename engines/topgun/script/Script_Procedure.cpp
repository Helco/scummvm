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
#define TOPGUN_SCRIPT_OP(name) #name
#include "ScriptOps.h"
#undef TOPGUN_SCRIPT_OP
};

typedef int64 TopGunFixedPoint;
constexpr TopGunFixedPoint FPOne = 10000;

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
	case ScriptOp::kGetWindowsVersion:
		return 95; // this might have to be changed per-game?
	case ScriptOp::kIsSingleGameOpen:
		return true;
	case ScriptOp::kAssert:
		checkArgCount(argCount, 2);
		if (!args[1])
			error("Script assertion failed (%d): %s", args[1], getString(args[0]).c_str());
		break;
	case ScriptOp::kMessageBox:
		checkArgCount(argCount, 1);
		// there is a fixed title "Studio7", not terribly important
		g_system->messageBox(LogMessageType::kInfo, getString(args[0]).c_str());
		break;

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
	case ScriptOp::kSetOnKeyDown:
	case ScriptOp::kSetOnKeyDown_dup:
		checkArgCount(argCount, 1);
		_keyDownEventHandler = args[0];
		break;
	case ScriptOp::kSetCursor:
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setCursor((CursorType)args[0]);
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
	case ScriptOp::kLoadPaletteResource:
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setPaletteFromResource(args[0]);
		break;
	case ScriptOp::kGetFreeGlobalMemory:
		// seems to be used for compatibility checks so any number higher is alright
		return INT32_MAX;
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
	case ScriptOp::kSpriteBreakLoops:
		checkArgCount(argCount, 2);
		_engine->loadResource<Sprite>(args[0])->setBreakLoops(args[1]);
		break;
	case ScriptOp::kSpritePause:
		checkArgCount(argCount, 2);
		if (args[0] == 0) {
			_engine->getSpriteCtx()->pause(args[1]);
			// TODO: Pause movies
		}
		else if (_engine->isResourceLoaded(args[0]))
			_engine->loadResource<Sprite>(args[0])->pause(args[1]);
		break;
	case ScriptOp::kSpriteSetPos:
		checkArgCount(argCount, 3);
		_engine->loadResource<Sprite>(args[0])->translate(Point(args[1], args[2]), false);
		break;
	case ScriptOp::kSpriteGetPos: {
		checkArgCount(argCount, 3);
		const auto pos = _engine->loadResource<Sprite>(args[0])->getPos();
		setVariable(args[1], pos.x);
		setVariable(args[2], pos.y);
	}break;
	case ScriptOp::kSpriteGetBounds:
	case ScriptOp::kSpriteGetBounds_dup: {
		checkArgCount(argCount, 5);
		const auto bounds = _engine->loadResource<Sprite>(args[0])->getBounds();
		setVariable(args[1], bounds.left);
		setVariable(args[2], bounds.top);
		setVariable(args[3], bounds.right);
		setVariable(args[4], bounds.bottom);
	}break;
	case ScriptOp::kSpriteGetNumCells:
		checkArgCount(argCount, 1);
		return (int32)_engine->loadResource<Sprite>(args[0])->getCellCount();
	case ScriptOp::kSpriteIsVisible:
		checkArgCount(argCount, 1);
		return _engine->isResourceLoaded(args[0]) && _engine->loadResource<Sprite>(args[0])->isVisible();

	case ScriptOp::kLoadResource:
		checkArgCount(argCount, 1);
		_engine->loadResource(args[0], ResourceType::kInvalid);
		return 1;
	case ScriptOp::kFreeResource:
	case ScriptOp::kFreeResource_dup: {
		checkArgCount(argCount, 1);
		auto wasLoaded = _engine->isResourceLoaded(args[0]);
		_engine->freeResource(args[0]);
		return wasLoaded;
	}
	case ScriptOp::kIsResourceLoaded:
		checkArgCount(argCount, 1);
		return _engine->isResourceLoaded(args[0]);
	case ScriptOp::kCopyResource:
		checkArgCount(argCount, 1);
		return _engine->copyResource(args[0], ResourceType::kInvalid)->getResourceIndex();
	case ScriptOp::kBackupAdditionalHMMIO:
		// This probably was used to save disk space by only copying
		// necessary data files from CD to harddisk?
		// Anyway not needed for ScummVM
		break;

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
		checkArgCount(argCount, 1);
		pauseTimers(args[0]);
		break;
	case ScriptOp::kSetNoInputScript:
		checkArgCount(argCount, 2);
		_engine->setNoInputScript(args[0], args[1] * 1000);
		break;
	case ScriptOp::kGetSecondsSinceNoInput:
		return (g_system->getMillis() - _engine->getNoInputLastEventTime()) / 1000;
	case ScriptOp::kSetSecondsSinceNoInput:
		checkArgCount(argCount, 1);
		_engine->setNoInputLastEventTime(g_system->getMillis() - args[0] * 1000);
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
	case ScriptOp::kSpriteGetScrollBox: {
		checkArgCount(argCount, 4);
		const auto rect = _engine->getSpriteCtx()->getScrollBox();
		setVariable(args[0], rect.left);
		setVariable(args[1], rect.top);
		setVariable(args[2], rect.right);
		setVariable(args[3], rect.bottom);
	}break;
	case ScriptOp::kSpriteGetScrollPos: {
		checkArgCount(argCount, 2);
		const auto pos = _engine->getSpriteCtx()->getScrollPos();
		setVariable(args[0], pos.x);
		setVariable(args[1], pos.y);
	}break;
	case ScriptOp::kGetResolution: {
		checkArgCount(argCount, 2);
		const auto fullBackgroundBounds = _engine->getSpriteCtx()->getFullBackgroundBounds();
		setVariable(args[0], fullBackgroundBounds.width());
		setVariable(args[1], fullBackgroundBounds.height());
		return _engine->getSpriteCtx()->isUsingBitmapBackground();
	}break;

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
	case ScriptOp::kGetMouseEventListener:
		return _mouseEventHandler;
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
		op._doDisable = !args[0];
		op._doEnable = args[0];
		setClickRect(op);
	}break;
	case ScriptOp::kToggleClickRect: {
		checkArgCount(argCount, 5);
		SetClickRectOp op;
		op._rect = Rect(args[0], args[1], args[2], args[3]);
		op._doDisable = !args[4];
		op._doEnable = args[4];
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
		op._doDisable = !args[1];
		op._doEnable = args[1];
		setClickRect(op);
	}break;
	case ScriptOp::kSpriteSetSync:
	case ScriptOp::kSpriteSetSyncInverted:
		// the sync flag seems to disable most of rendering for larger batches
		// of sprite modifications. We do not render upon changes so we can
		// ignore the sync flag entirely.
		break;

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
		const auto newValue = getString(args[4]);
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

	case ScriptOp::kAbsolute:
		checkArgCount(argCount, 1);
		return abs(args[0]);
	case ScriptOp::kMax:
		checkArgCount(argCount, 2);
		return MAX(args[0], args[1]);
	case ScriptOp::kMin:
		checkArgCount(argCount, 2);
		return MIN(args[0], args[1]);
	case ScriptOp::kFixedPointAdd: {
		checkArgCount(argCount, 4);
		const auto intVar = args[2];
		const auto fracVar = args[3];
		auto result = args[1] + evalValue(fracVar, true) + (args[0] + evalValue(intVar, true)) * FPOne;
		setVariable(intVar, (int32)(result / FPOne));
		setVariable(fracVar, (int32)(result % FPOne));
	}break;
	case ScriptOp::kFixedPointDiv: {
		checkArgCount(argCount, 4);
		const auto intVar = args[2];
		const auto fracVar = args[3];
		auto result = (evalValue(fracVar, true) + FPOne * evalValue(intVar, true)) / (args[1] + FPOne * args[0]);
		setVariable(intVar, (int32)(result / FPOne));
		setVariable(fracVar, (int32)(result % FPOne));
	}break;
	case ScriptOp::kFixedPointMul: {
		checkArgCount(argCount, 4);
		const auto intVar = args[2];
		const auto fracVar = args[3];
		auto result = (evalValue(fracVar, true) + FPOne * evalValue(intVar, true)) * (args[1] + FPOne * args[0]);
		setVariable(intVar, (int32)(result / FPOne));
		setVariable(fracVar, (int32)(result % FPOne));
	}break;

	case ScriptOp::kStringToInt:
		checkArgCount(argCount, 1);
		return atoi(getString(args[0]).c_str());
	case ScriptOp::kGetStringChar: {
		checkArgCount(argCount, 2);
		const auto string = getString(args[0]);
		return args[1] < 0 || args[1] >= string.size() ? 0 : string[args[1]];
	}break;
	case ScriptOp::kSetStringChar: {
		checkArgCount(argCount, 3);
		// in the original engine setting an index between 0 and 254 would always work
		// as strings there are of fixed size. Let's just check and error-out if this
		// actually happens
		if (isConstString(args[0]))
			break;
		auto string = getString(args[0]);
		if (args[1] < 0 || args[1] >= string.size())
			error("Tried to set string char at %d but string is only %d chars long", args[1], string.size());
		string.setChar(args[2], args[1]);
		setString(args[0], string);
	}break;
	case ScriptOp::kStringCompare:
		checkArgCount(argCount, 2);
		return getString(args[0]).compareTo(getString(args[1]));
	case ScriptOp::kStringCompareI:
		checkArgCount(argCount, 2);
		return getString(args[0]).compareToIgnoreCase(getString(args[1]));
	case ScriptOp::kStringLength:
		checkArgCount(argCount, 1);
		return getString(args[0]).size();
	case ScriptOp::kStringFind: {
		checkArgCount(argCount, 3);
		const auto haystack = getString(args[0]);
		if (args[2] < 0 || args[2] >= haystack.size())
			return -1;
		const auto index = getString(args[0]).find(getString(args[1]));
		return index == Common::String::npos ? -1 : index;
	}break;
	case ScriptOp::kStringConcat:
		checkArgCount(argCount, 2);
		setString(args[0], getString(args[0]) + getString(args[1]));
		break;
	case ScriptOp::kStringCopy:
		checkArgCount(argCount, 2);
		setString(args[0], getString(args[1]));
		break;
	case ScriptOp::kStringCopySized:
		checkArgCount(argCount, 3);
		setString(args[0], getString(args[1]).substr(0, args[2]));
		break;

	case ScriptOp::kAudioPlayWave146:
		warning("stub procedure AudioPlayWave146");
		break;
	case ScriptOp::kAudioStopWave:
		warning("stub procedure AudioStopWave");
		break;
	case ScriptOp::kAudioSetWaveSoundPriority:
		checkArgCount(argCount, 2);
		warning("stub procedure AudioSetWaveSoundPriority");
		break;
	case ScriptOp::kAudioGetWaveSoundTime:
		checkArgCount(argCount, 1);
		warning("stub procedure AudioGetWaveSoundTime");
		return 0;
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
		return true; // yes, this success-condition is weird.

	return _engine->loadResource<Sprite>(spriteIndex)->setQueue(queueIndex, hideSprite);
}

}

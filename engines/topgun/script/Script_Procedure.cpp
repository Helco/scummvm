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

namespace TopGun {

static void checkArgCount(uint32 actual, uint32 expected) {
	if (actual != expected)
		error("Invalid number of procedure arguments, expected %d but got %d", expected, actual);
}

static void checkArgCount(uint32 actual, uint32 min, uint32 max) {
	if (actual < min || actual > max)
		error("Invalid number of procedure arguments, expected %d-%d but got %d", min, max, actual);
}

int32 Script::runInternalProcedure(uint32 procId, const int32 *args, uint32 argCount) {
	switch ((ScriptOp)procId) {
	case ScriptOp::kSetScriptReg3E3F:
		checkArgCount(argCount, 1);
		_reg3E3F = args[0];
		break;
	case ScriptOp::kSetCursor:
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setCursor(args[0]);
		break;
	case ScriptOp::kStopFade:
		// TODO: Implement, was postponed because non-essential
		break;
	case ScriptOp::kGetFreeGlobalMemory:
		// seems to be used for compatibility checks so any number higher is alright
		return INT32_MAX;
	case ScriptOp::kClearTopMostSpriteNextFrame:
		checkArgCount(argCount, 1);
		warning("stub procedure ClearTopMostSpriteNextFrame");
		break;
	case ScriptOp::kLoadResource:
		checkArgCount(argCount, 1);
		_engine->loadResource(args[0], ResourceType::kInvalid);
		break;
	case ScriptOp::kSetPauseEventScript:
		checkArgCount(argCount, 1);
		_pauseEventHandler = args[0];
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
	default:
		error("Unknown or unimplemented internal procedure: %d", procId);
	}

	return static_cast<int32>(procId);
}

}

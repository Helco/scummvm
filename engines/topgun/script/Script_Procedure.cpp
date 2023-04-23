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

int32 Script::runInternalProcedure(uint32 procId, const int32 *args, uint32 argCount) {
	switch ((ScriptOp)procId) {
	case ScriptOp::kSetCursor:
		checkArgCount(argCount, 1);
		_engine->getSpriteCtx()->setCursor(args[0]);
		break;
	case ScriptOp::kStopFade:
		// TODO: Implement, was postponed because non-essential
		break;
	default:
		error("Unknown or unimplemented internal procedure: %d", procId);
	}

	return static_cast<int32>(procId);
}

}

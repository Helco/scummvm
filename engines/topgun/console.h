
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

#ifndef TOPGUN_CONSOLE_H
#define TOPGUN_CONSOLE_H

#include "gui/debugger.h"

namespace TopGun {

class TopGunEngine;
class ScriptDebugger;

class Console : public GUI::Debugger {
private:
	TopGunEngine *_engine;
	ScriptDebugger *_scriptDebugger;

	bool Cmd_addPoint(int argc, const char **argv);
	bool Cmd_removePoint(int argc, const char **argv);
	bool Cmd_removeAllPoints(int argc, const char **argv);
	bool Cmd_continue(int argc, const char **argv);
	bool Cmd_step(int argc, const char **argv);
	bool Cmd_stepOver(int argc, const char **argv);
	bool Cmd_stepOut(int argc, const char **argv);
	bool Cmd_listPoints(int argc, const char **argv);
	bool Cmd_stacktrace(int argc, const char **argv);
	bool Cmd_localVars(int argc, const char **argv);
	bool Cmd_globalVars(int argc, const char **argv);
	bool Cmd_dynStrings(int argc, const char **argv);
public:
	Console(TopGunEngine *engine);
	~Console() override;
};

} // End of namespace Topgun

#endif // TOPGUN_CONSOLE_H


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

#ifndef EDNA_CONSOLE_H
#define EDNA_CONSOLE_H

#include "edna/util.h"
#include "gui/debugger.h"

namespace Edna {

class Game;

class Console final : public GUI::Debugger {
public:
	Console();
	~Console() override;

	bool hasBreakpoint(ScriptId scriptId, uint32 line) const;
private:
	bool cmdValidate(int argc, const char **argv);
	bool cmdRoom(int argc, const char **argv);
	bool cmdSprites(int argc, const char **argv);
	bool cmdScript(int argc, const char **argv);
	bool cmdEval(int argc, const char **argv);
	bool cmdRun(int argc, const char **argv);
	bool cmdBreakpoint(int argc, const char **argv);
	bool cmdDelBreakpoint(int argc, const char **argv);

	Game *getGame();
	bool tryParseUint(const char *arg, uint32 &value, const char *context);

	using BreakpointList = Common::SortedArray<TwoKey, const TwoKey &>;
	BreakpointList::const_iterator getBreakpoint(uint32 scriptId, uint32 line) const;
	void printBreakpointList();
	BreakpointList _breakpoints;
};

} // End of namespace Edna

#endif // EDNA_CONSOLE_H

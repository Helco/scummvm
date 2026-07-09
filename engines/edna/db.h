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

#ifndef EDNA_DB_H
#define EDNA_DB_H

#include "common/span.h"
#include "common/str.h"

namespace Edna {

using FileData = Common::SpanOwner<Common::Span<char>>;

struct ScriptLine {
	uint32 _script = 0;
	uint32 _line = 0;
	const char *_command = nullptr;
	const char *_comment = nullptr;
};

class DB final {
public:
	DB(const Common::Path &path);
	~DB();

	Common::Span<const ScriptLine> script(uint scriptId) const;

private:
	void loadScripts(const Common::Path &path);

	struct Script {
		uint32 _begin = 0;
		uint32 _count = 0;
	};
	FileData _scriptData;
	Common::Array<ScriptLine> _scriptLines;
	Common::HashMap<uint32, Script> _scripts;
};

}

#endif // EDNA_DB_H

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

#include "common/file.h"

#include "db.h"

using namespace Common;

namespace Edna {

/* The database in Edna is originally HSQLDB with the data stored in CSV files
 * We use the files in-place, adding zero-terminators where necessary.
 * A quirk is that lines may stretch across multiple lines, only the last
 * column is finished by a newline. 
 */

static void loadFile(FileData &data, const Path &basePath, const char *fileName) {
	Path path = basePath.appendComponent(fileName);
	File file;
	if (!file.open(path)) {
		error("Could not open data file: %s", path.toString().c_str());
		return;
	}
	data->allocate((uint32)(file.size() + 1));
	if (file.read(data->data(), data->size() - 1) != data->size() - 1) {
		error("Could not read data file: %s", path.toString().c_str());
		return;
	}
	data[data->size() - 1] = '\0';
}

static void skipWhitespace(char *&full) {
	while (*full && isSpace(*full))
		full++;
}

// the returned span is also null-terminated
static Span<char> nextCell(char *&full, bool isLastColumn = false) {
	char *const cell = full;
	uint32 length = 0;
	bool containsNewLine = false;
	while (*full) {
		if (isLastColumn) {
			if (*full == '\n') {
				if (length > 0 && full[-1] == '\r') {
					length--;
					full[-1] = '\0';
				}
				*full++ = '\0';
				break;
			}
		} else {
			if (*full == ';') {
				*full++ = '\0';
				break;
			} else if (*full == '\n')
				containsNewLine = true;
		}
		length++;
		full++;
	}
	if (!containsNewLine)
		return Span<char>(cell, length);

	// if there are newlines there might be carriage returns.
	// those have to be removed
	char *cursor = cell;
	char *cr = strchr(cursor, '\r');
	while (cr != nullptr) {
		length--;
		memmove(cr, cr + 1, cell + length - cr);
		cursor = cr;
		cr = strchr(cursor, '\r');
	}
	return Span<char>(cell, length);
}

static char *nextString(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	return cell.data();
}

static uint32 nextInteger(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	char *end = nullptr;
	auto value = strtoul(cell.data(), &end, 10);
	if (end == nullptr || *end != '\0')
		error("Could not extract integer from data file");
	return (uint32)value;
}

static float nextFloat(char *&full, bool isLastColumn = false) {
	auto cell = nextCell(full, isLastColumn);
	char *end = nullptr;
	float value = strtof(cell.data(), &end);
	if (end == nullptr || *end != '\0')
		error("Could not extract float from data file");
	return (uint32)value;
}

DB::DB(const Path &path) {
	loadScripts(path);
}

DB::~DB() {}

void DB::loadScripts(const Path &path) {
	loadFile(_scriptData, path, "skript.csv");

	char *full = _scriptData->data();
	skipWhitespace(full);
	while (*full) {
		ScriptLine scriptLine;
		scriptLine._script = nextInteger(full);
		scriptLine._line = nextInteger(full);
		scriptLine._command = nextString(full);
		scriptLine._comment = nextString(full, true);
		_scriptLines.push_back(scriptLine);
		skipWhitespace(full);
	}

	sort(_scriptLines.begin(), _scriptLines.end(), [&](const ScriptLine &a, const ScriptLine &b) {
		return a._script != b._script
			? a._script < b._script
			: a._line < b._line;
	});

	uint32 scriptBegin = 0;
	for (uint32 i = 1; i < _scriptLines.size(); i++) {
		if (_scriptLines[scriptBegin]._script != _scriptLines[i]._script) {
			_scripts[_scriptLines[scriptBegin]._script] = { scriptBegin, i - scriptBegin };
			scriptBegin = i;
		}
	}
	_scripts[_scriptLines[scriptBegin]._script] = { scriptBegin, _scriptLines.size() - scriptBegin };
}

}

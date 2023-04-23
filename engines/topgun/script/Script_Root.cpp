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

using Common::String;

namespace TopGun {

void Script::runSingleRootInstruction(Common::MemorySeekableReadWriteStream &stream) {
	const auto op = (ScriptOp)stream.readUint16LE();
	debugCN(kSuperVerbose, kDebugScript, "root instruction %d\n", op);
	switch (op) {
	case ScriptOp::kSetCursor:
		_engine->getSpriteCtx()->setCursor(stream.readSint16LE());
		break;
	case ScriptOp::kJumpIfCalc: {
		const auto startPosition = stream.pos();
		const int32 elseDistance = readSint(stream);
		const int32 thenDistance = readSint(stream);
		if (runCalc(stream))
			stream.seek(startPosition + thenDistance - 2, SEEK_SET);
		else
			stream.seek(startPosition + elseDistance - 2, SEEK_SET);
	}break;
	case ScriptOp::kRunCalc: {
		auto calcStream = stream.readStream(readUint(stream) - calcJumpOffset(1));
		runCalc(*calcStream);
		delete calcStream;
	}break;
	case ScriptOp::kSetString: {
		const auto targetString = readSint(stream);
		auto formatString = readSint(stream);
		stream.skip(1);
		formatString = evalValue(formatString, stream.readByte() != 0);

		const auto formatCount = readUint(stream);
		constexpr size_t kMaxFormats = 6;
		if (formatCount > kMaxFormats)
			error("Too many format values declared for SetString operation");
		Array<FormatValue> formatValues(formatCount);
		size_t i;
		for (i = 0; i < formatCount; i++) {
			const auto valueOrIndex = readSint(stream);
			formatValues[i]._isInteger = stream.readByte() != 0;
			const auto isIndirect = stream.readByte() != 0;
			if (formatValues[i]._isInteger)
				formatValues[i]._integer = evalValue(valueOrIndex, isIndirect);
			else
				formatValues[i]._string = getString(valueOrIndex);
		}
		for (; i < kMaxFormats; i++) {
			readSint(stream);
			stream.skip(2);
		}

		const auto resultString = sprintfWithArray(getString(formatString), formatValues);
		setString(targetString, resultString);
	}break;

	default: error("Unknown or unimplemented root script instruction: %d", op);
	}
}

const char *walkOverFormatSpecifier(const char *format) {
	// according to https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-wsprintfa
	// we just have to look for certain letters to find the end of the format specifier
	constexpr const char *kEndCharacters = "%cCdsSuixXp";
	if (format == nullptr)
		return nullptr;
	while (*format && strchr(kEndCharacters, *format) == nullptr)
		format++;
	return format;
}

String Script::sprintfWithArray(const Common::String &format, const Array<FormatValue> &values) {
	// the original game just used wvsprintfA and created a va_list
	// But this is undefined behaviour

	auto formatPtr = format.c_str();
	String result = "";
	auto itValue = values.begin();
	const char *endOfSpecifier;
	while (true) {
		endOfSpecifier = walkOverFormatSpecifier(strchr(formatPtr, '%'));
		if (endOfSpecifier == nullptr || *endOfSpecifier == 0)
			break;
		else if (*endOfSpecifier == '%') {
			result += Common::String(formatPtr, endOfSpecifier);
			formatPtr = endOfSpecifier + 1;
		}
		// now we have an actual specifier to replace
		else if (itValue == values.end())
			error("Too few replacement values for the format string");
		else {
			String subFormat(formatPtr, endOfSpecifier + 1);
			result += itValue->_isInteger
				? String::format(subFormat.c_str(), itValue->_string.c_str())
				: String::format(subFormat.c_str(), itValue->_integer);
			itValue++;
		}
	}
	if (*formatPtr != 0)
		result += formatPtr;
	if (itValue != values.end())
		error("Too many replacement values for the format string");
	return result;
}

}

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

void Script::runSingleRootInstruction(Common::MemorySeekableReadWriteStream &stream, uint32 scriptIndex) {
	const auto op = (ScriptOp)stream.readUint16LE();
	debugCN(kSuperVerbose, kDebugScript, "root instruction %d\n", op);
	switch (op) {
	case ScriptOp::kNop: break;
	case ScriptOp::kSetCursor:
		_engine->getSpriteCtx()->setCursor(stream.readSint16LE());
		break;
	case ScriptOp::kJumpIf: {
		const auto startPosition = stream.pos();
		const auto distance = readSint(stream);
		auto left = readSint(stream);
		auto right = readSint(stream);
		const auto subOp = stream.readByte();
		left = evalValue(left, stream.readByte());
		right = evalValue(right, stream.readByte());
		stream.skip(1);
		if (simpleCondition(left, right, subOp))
			stream.seek(startPosition + distance - 2, SEEK_SET);
	}break;
	case ScriptOp::kJumpIfCalc:
	case ScriptOp::kJumpIfCalc_dup: {
		const auto startPosition = stream.pos();
		const int32 elseDistance = readSint(stream);
		const int32 thenDistance = readSint(stream);
		if (runCalc(stream, scriptIndex))
			stream.seek(startPosition + thenDistance - 2, SEEK_SET);
		else
			stream.seek(startPosition + elseDistance - 2, SEEK_SET);
	}break;
	case ScriptOp::kJump:
		stream.seek(readSint(stream) - calcJumpOffset(1), SEEK_CUR);
		break;
	case ScriptOp::kReturn:
		readUint(stream);
		_scriptResult = runCalc(stream, scriptIndex);
		stream.seek(0, SEEK_END);
		break;
	case ScriptOp::kExit:
		stream.seek(0, SEEK_END);
		break;
	case ScriptOp::kRunCalc: {
		const auto endPosition = stream.pos() - 2 + readUint(stream);
		runCalc(stream, scriptIndex);
		stream.seek(endPosition, SEEK_SET);
	}break;
	case ScriptOp::kSimpleCalc: {
		constexpr uint32 kMaxOpCount = 3;
		auto targetIndex = readSint(stream);
		auto targetValue = evalValue(targetIndex, true);
		auto opCount = readUint(stream);
		for (uint i = 0; i < kMaxOpCount; i++) {
			const auto right = readSint(stream);
			const auto subOp = stream.readByte();
			stream.skip(1);
			const auto negateRight = stream.readByte() != 0;
			const auto isRightIndirect = stream.readByte() != 0;
			if (i < opCount)
				targetValue = simpleCalc(targetValue, right, subOp, negateRight, isRightIndirect);
		}
		setVariable(targetIndex, targetValue);
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
	case ScriptOp::kSwitch: {
		const auto startPos = stream.pos();
		auto value = readSint(stream);
		const auto offsetToCases = readUint(stream);
		const auto defaultJumpDistance = readSint(stream);
		const auto caseCount = stream.readUint16LE();
		stream.skip(1);
		value = evalValue(value, stream.readByte() != 0);
		jumpToCase(stream, value, offsetToCases, caseCount, defaultJumpDistance, startPos);
	}break;
	case ScriptOp::kCalcSwitch: {
		const auto startPos = stream.pos();
		readUint(stream);
		const auto offsetToCases = readUint(stream);
		const auto defaultJumpDistance = readSint(stream);
		const auto caseCount = stream.readUint16LE();
		const auto result = runCalc(stream, scriptIndex);
		jumpToCase(stream, result, offsetToCases, caseCount, defaultJumpDistance, startPos);
	}break;

	default: error("Unknown or unimplemented root script instruction: %d", op);
	}
}

void Script::jumpToCase(Common::SeekableReadStream &stream,
						int32 switchValue,
						uint32 offsetToCases,
						uint32 caseCount,
						int32 defaultJumpDistance,
						int64 startPos) {
	stream.seek(startPos + offsetToCases - 2, SEEK_SET);
	auto jumpDistance = defaultJumpDistance;
	for (uint32 i = 0; i < caseCount; i++) {
		stream.skip(2); // this proabably should always be ScriptOp::kCase
		auto caseValue = readSint(stream);
		const auto caseJumpDistance = readSint(stream);
		caseValue = evalValue(caseValue, stream.readByte() != 0);
		stream.skip(1);
		if (caseValue == switchValue) {
			jumpDistance = caseJumpDistance;
			break;
		}
	}
	stream.seek(startPos + jumpDistance - 2, SEEK_SET);
}

int32 Script::simpleCalc(int32 left, int32 right, byte op, bool negateRight, bool isRightIndirect) {
	right = evalValue(right, isRightIndirect);
	if (negateRight)
		right = -right;
	switch (op) {
	case 0: return right;
	case 1: return left + right;
	case 2: return left - right;
	case 3: return left * right;
	case 4: return (left + abs(right / 2)) / right;
	case 5: return left | right;
	case 6: return left & right;
	case 7: return left % right;
	default: return 0;
	}
}

bool Script::simpleCondition(int32 left, int32 right, byte op) {
	bool result;
	if (op & (1 << 0))
		result = left == right;
	else if (op & (1 << 1))
		result = left > right;
	else if (op & (1 << 2))
		result = left < right;
	else if (op & (1 << 3))
		result = left | right;
	else if (op & (1 << 4))
		result = left & right;
	else if (op & (1 << 5))
		result = left % right;
	else
		result = left != 0;

	if (op & (1 << 7))
		result = !result;
	return result;
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
	// which is undefined behaviour

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

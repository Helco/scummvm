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

int32 Script::runCalc(Common::SeekableReadStream &stream, uint32 callingScriptIndex) {
	_debugger->onCallStart(ScriptCallType::kCalc, callingScriptIndex, stream.pos());
	const auto prevStackSize = _stack.size();
	auto op = (ScriptCalcOp)stream.readByte();
	while (op != ScriptCalcOp::kExit && !stream.err()) {
		debugCN(kSuperVerbose, kDebugScript, "calc instruction: %d\n", op); // TODO: move into script debugger
		switch (op) {
		case ScriptCalcOp::kPushValue:
			stackPush(readSint(stream));
			break;
		case ScriptCalcOp::kPushVar:
			stackPush(evalValue(readSint(stream), true));
			break;
		case ScriptCalcOp::kPushVarValue:
			stackPush(evalValue(stackTop(), true));
			break;
		case ScriptCalcOp::kReadVarArray: {
			const auto index = stackPop();
			const auto array = stackPop();
			stackPush(evalValue(index + array, true));
		}break;
		case ScriptCalcOp::kPushVarAddress:
			/* Originally the memory address to variables are also pushed on the stack
			 * This seems unsafe so let's just keep working with variable indices
			 */
			stackPush(readSint(stream));
			break;
		case ScriptCalcOp::kReadVar:
			stackPush(evalValue(stackPop(), true));
			break;
		case ScriptCalcOp::kOffsetVar: {
			const auto offset = stackPop();
			const auto base = stackPop();
			stackPush(base + offset);
		}break;
		case ScriptCalcOp::kWriteVar: {
			const auto value = stackPop();
			const auto variable = stackPop();
			setVariable(variable, value);
			stackPush(value);
		}break;
		case ScriptCalcOp::kCallProc: {
			const auto scopeSize = readUint(stream);
			const auto argCount = readUint(stream);
			const auto procId = _stack[_stack.size() - argCount - 1];

			const auto result = runProcedure(procId, _stack.data() + _stack.size() - argCount, argCount, scopeSize);

			_stack.resize(_stack.size() - argCount - 1);
			stackPush(result);
		}break;
		case ScriptCalcOp::kRunScript: {
			const auto scopeSize = readUint(stream);
			const auto argCount = readUint(stream);
			const auto scriptIndex = _stack[_stack.size() - argCount - 1];

			const auto result = runMessage(scriptIndex, scopeSize, argCount, _stack.data() + _stack.size() - argCount);

			_stack.resize(_stack.size() - argCount - 1);
			stackPush(result);
		}break;
		case ScriptCalcOp::kNegate: stackPush(-stackPop()); break;
		case ScriptCalcOp::kBooleanNot: stackPush(stackPop() == 0); break;
		case ScriptCalcOp::kBitNot: stackPush(~stackPop()); break;
		case ScriptCalcOp::kAdd: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left + right);
		}break;
		case ScriptCalcOp::kSub: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left - right);
		}break;
		case ScriptCalcOp::kMul: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left * right);
		}break;
		case ScriptCalcOp::kDiv: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left / right);
		}break;
		case ScriptCalcOp::kMod: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left % right);
		}break;
		case ScriptCalcOp::kEquals: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left == right);
		}break;
		case ScriptCalcOp::kNotEquals: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left != right);
		}break;
		case ScriptCalcOp::kLessOrEquals: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left <= right);
		}break;
		case ScriptCalcOp::kLess: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left < right);
		}break;
		case ScriptCalcOp::kGreaterOrEquals: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left >= right);
		}break;
		case ScriptCalcOp::kGreater: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left > right);
		}break;
		case ScriptCalcOp::kBooleanAnd: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left != 0 && right != 0);
		}break;
		case ScriptCalcOp::kBooleanOr: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left != 0 || right != 0);
		}break;
		case ScriptCalcOp::kBitAnd: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left & right);
		}break;
		case ScriptCalcOp::kBitOr: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left | right);
		}break;
		case ScriptCalcOp::kBitXor: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left ^ right);
		}break;
		case ScriptCalcOp::kShiftLeft: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left << right);
		}break;
		case ScriptCalcOp::kShiftRight: {
			const auto right = stackPop();
			const auto left = stackPop();
			stackPush(left >> right);
		}break;
		case ScriptCalcOp::kPreIncrementVar: {
			const auto variable = stackPop();
			auto varValue = evalValue(variable, true);
			stackPush(++varValue);
			setVariable(variable, varValue);
		}break;
		case ScriptCalcOp::kPostIncrementVar: {
			const auto variable = stackPop();
			auto varValue = evalValue(variable, true);
			stackPush(varValue++);
			setVariable(variable, varValue);
		}break;
		case ScriptCalcOp::kPreDecrementVar: {
			const auto variable = stackPop();
			auto varValue = evalValue(variable, true);
			stackPush(--varValue);
			setVariable(variable, varValue);
		}break;
		case ScriptCalcOp::kPostDecrementVar: {
			const auto variable = stackPop();
			auto varValue = evalValue(variable, true);
			stackPush(varValue--);
			setVariable(variable, varValue);
		}break;
		case ScriptCalcOp::kJumpZero: {
			const auto jumpTarget = stream.pos() + readSint(stream);
			if (!stackTop()) {
				stream.seek(jumpTarget, SEEK_SET);
				stackPush(0);
			}
		}break;
		case ScriptCalcOp::kJumpNonZero: {
			const auto jumpTarget = stream.pos() + readSint(stream);
			if (stackTop()) {
				stream.seek(jumpTarget, SEEK_SET);
				stackPush(1);
			}
		}break;
		default:
			error("Unknown calc script op: %d", op);
		}
		_debugger->onCallIncrement(stream.pos());
		op = (ScriptCalcOp)stream.readByte();
	}
	if (stream.err())
		error("Stream error during calc script execution");

	if (prevStackSize >= _stack.size())
		error("Invalid stack state after calc script");
	const auto result = _stack[prevStackSize];
	_stack.resize(prevStackSize);
	_debugger->onCallEnd();
	return result;
}

}

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

#include "script.h"
#include "rooms.h"
#include "alcachofa.h"

#include "common/file.h"

using namespace Common;
using namespace Math;

namespace Alcachofa {

ScriptInstruction::ScriptInstruction(ReadStream &stream)
	: _op((ScriptOp)stream.readSint32LE())
	, _arg(stream.readSint32LE()) {}

Script::Script() {
	File file;
	if (!file.open("script/SCRIPT.COD"))
		error("Could not open script");

	uint32 stringBlobSize = file.readUint32LE();
	uint32 memorySize = file.readUint32LE();
	_strings = SpanOwner<Span<char>>({ new char[stringBlobSize], stringBlobSize });
	if (file.read(&_strings[0], stringBlobSize) != stringBlobSize)
		error("Could not read script string blob");
	if (_strings[stringBlobSize - 1] != 0)
		error("String blob does not end with null terminator");

	if (memorySize % sizeof(int32) != 0)
		error("Unexpected size of script memory");
	_variables.resize(memorySize / sizeof(int32), 0);

	uint32 variableCount = file.readUint32LE();
	for (uint32 i = 0; i < variableCount; i++) {
		String name = readVarString(file);
		uint32 offset = file.readUint32LE();
		if (offset % sizeof(int32) != 0)
			error("Unaligned variable offset");
		_variableNames[name] = offset / 4;
	}

	uint32 procedureCount = file.readUint32LE();
	for (uint32 i = 0; i < procedureCount; i++) {
		String name = readVarString(file);
		uint32 offset = file.readUint32LE();
		file.skip(sizeof(uint32));
		_procedures[name] = offset - 1; // originally one-based, but let's not.
	}

	uint32 behaviorCount = file.readUint32LE();
	for (uint32 i = 0; i < behaviorCount; i++) {
		String behaviorName = readVarString(file) + '/';
		variableCount = file.readUint32LE(); // not used by the original game
		assert(variableCount == 0);
		procedureCount = file.readUint32LE();
		for (uint32 j = 0; j < procedureCount; j++) {
			String name = behaviorName + readVarString(file);
			uint32 offset = file.readUint32LE();
			file.skip(sizeof(uint32));
			_procedures[name] = offset - 1;
		}
	}

	uint32 instructionCount = file.readUint32LE();
	_instructions.reserve(instructionCount);
	for (uint32 i = 0; i < instructionCount; i++)
		_instructions.push_back(ScriptInstruction(file));
}

int32 Script::variable(const char *name) const {
	uint32 index;
	if (!_variableNames.tryGetVal(name, index))
		error("Unknown variable: %s", name);
	return _variables[index];
}

int32 &Script::variable(const char *name) {
	uint32 index;
	if (!_variableNames.tryGetVal(name, index))
		error("Unknown variable: %s", name);
	return _variables[index];
}

bool Script::hasProcedure(const Common::String &behavior, const Common::String &action) const {
	return hasProcedure(behavior + '/' + action);
}

bool Script::hasProcedure(const Common::String &procedure) const {
	return _procedures.contains(procedure);
}

struct ScriptTimerTask : public Task {
	ScriptTimerTask(Process &process, int32 durationSec)
		: Task(process)
		, _durationSec(durationSec) {}

	virtual TaskReturn run() override {
		TASK_BEGIN;
		if (_durationSec >= (int32)((g_system->getMillis() - g_engine->script()._scriptTimer) / 1000) &&
			g_engine->script().variable("SeHaPulsadoRaton"))
			_result = 0;
		
		// TODO: Add network behavior for script timer
		TASK_YIELD;
		TASK_END;
	}

	virtual void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("Check input timer for %dsecs", _durationSec);
	}

private:
	int32 _durationSec;
	int32 _result = 1;
};

enum class StackEntryType {
	Number,
	Variable,
	String,
	Instruction
};

struct StackEntry {
	StackEntry(StackEntryType type, int32 number) : _type(type), _number(number) {}
	StackEntry(StackEntryType type, uint32 index) : _type(type), _index(index) {}

	StackEntryType _type;
	union {
		int32 _number;
		uint32 _index;
	};
};

struct ScriptTask : public Task {
	ScriptTask(Process &process, const String &name, uint32 pc, FakeLock &&lock)
		: Task(process)
		, _script(g_engine->script())
		, _name(name)
		, _pc(pc)
		, _lock(Common::move(lock)) {
		pushInstruction(UINT_MAX);
	}

	ScriptTask(Process &process, const ScriptTask &forkParent)
		: Task(process)
		, _script(g_engine->script())
		, _name(forkParent._name + " FORKED")
		, _pc(forkParent._pc)
		, _lock(forkParent._lock) {
		for (uint i = 0; i < forkParent._stack.size(); i++)
			_stack.push(forkParent._stack[i]);
		pushNumber(1); // this task is the forked one
	}

	virtual TaskReturn run() override {
		if (_isFirstExecution || _returnsFromKernelCall)
			setCharacterVariables();
		if (_returnsFromKernelCall)
			pushNumber(process().returnValue());
		_isFirstExecution = _returnsFromKernelCall = false;

		while (true) {
			if (_pc >= _script._instructions.size())
				error("Script process reached instruction out-of-bounds");
			const auto &instruction = _script._instructions[_pc++];
			switch (instruction._op) {
			case ScriptOp::Nop: break;
			case ScriptOp::Dup:
				if (_stack.empty())
					error("Script tried to duplicate stack top, but stack is empty");
				_stack.push(_stack.top());
				break;
			case ScriptOp::PushAddr:
				pushVariable(instruction._arg);
				break;
			case ScriptOp::PushValue:
				pushNumber(instruction._arg);
				break;
			case ScriptOp::Deref:
				pushNumber(popVariable());
				break;
			case ScriptOp::PopN:
				if (instruction._arg < 0 || (uint)instruction._arg > _stack.size())
					error("Script tried to pop more entries than are available on the stack");
				for (int32 i = 0; i < instruction._arg; i++)
					_stack.pop();
				break;
			case ScriptOp::Store: {
				int32 value = popNumber();
				popVariable() = value;
				pushNumber(value);
			}break;
			case ScriptOp::LoadString:
			case ScriptOp::LoadString2:
				pushString(popNumber());
				break;
			case ScriptOp::ScriptCall:
				pushInstruction(_pc);
				_pc = instruction._arg - 1;
				break;
			case ScriptOp::KernelCall: {
				TaskReturn kernelReturn = kernelCall((ScriptKernelTask)instruction._arg);
				if (kernelReturn.type() == TaskReturnType::Waiting) {
					_returnsFromKernelCall = true;
					return kernelReturn;
				}
				else
					pushNumber(kernelReturn.returnValue());
			}break;
			case ScriptOp::JumpIfFalse:
				if (popNumber() == 0)
					_pc = _pc - 1 + instruction._arg;
				break;
			case ScriptOp::JumpIfTrue:
				if (popNumber() != 0)
					_pc = _pc - 1 + instruction._arg;
				break;
			case ScriptOp::Jump:
				_pc = _pc - 1 + instruction._arg;
				break;
			case ScriptOp::Negate:
				pushNumber(-popNumber());
				break;
			case ScriptOp::BooleanNot:
				pushNumber(popNumber() == 0 ? 1 : 0);
				break;
			case ScriptOp::Mul:
				pushNumber(popNumber() * popNumber());
				break;
			case ScriptOp::Add:
				pushNumber(popNumber() + popNumber());
				break;
			// flipped operators to not use a temporary
			case ScriptOp::Sub:
				pushNumber(-popNumber() + popNumber());
				break;
			case ScriptOp::Less:
				pushNumber(popNumber() >= popNumber());
				break;
			case ScriptOp::Greater:
				pushNumber(popNumber() <= popNumber());
				break;
			case ScriptOp::LessEquals:
				pushNumber(popNumber() >= popNumber());
				break;
			case ScriptOp::GreaterEquals:
				pushNumber(popNumber() <= popNumber());
				break;
			case ScriptOp::Equals:
				pushNumber(popNumber() == popNumber());
				break;
			case ScriptOp::NotEquals:
				pushNumber(popNumber() != popNumber());
				break;
			case ScriptOp::BitAnd:
				pushNumber(popNumber() & popNumber());
				break;
			case ScriptOp::BitOr:
				pushNumber(popNumber() | popNumber());
				break;
			case ScriptOp::Return: {
				int32 returnValue = popNumber();
				_pc = popInstruction();
				if (_pc == UINT_MAX)
					return TaskReturn::finish(returnValue);
				else
					pushNumber(returnValue);
			}break;
			case ScriptOp::Crash5:
			case ScriptOp::Crash8:
			case ScriptOp::Crash9:
			case ScriptOp::Crash12:
			case ScriptOp::Crash21:
			case ScriptOp::Crash22:
			case ScriptOp::Crash33:
			case ScriptOp::Crash34:
			case ScriptOp::Crash35:
			case ScriptOp::Crash36:
				error("Script reached crash instruction");
			default:
				error("Script reached invalid instruction");
			}
		}
	}

	virtual void debugPrint() {
		g_engine->getDebugger()->debugPrintf("\"%s\" at %u\n", _name.c_str(), _pc);
	}

private:
	void setCharacterVariables() {
		_script.variable("m_o_f") = (int32)process().character();
		_script.variable("m_o_f_real") = (int32)g_engine->player().activeCharacterKind();
	}

	void pushNumber(int32 value) {
		_stack.push({ StackEntryType::Number, value });
	}

	void pushVariable(uint32 offset) {
		uint32 index = offset / sizeof(int32);
		if (offset % sizeof(int32) != 0 || index >= _script._variables.size())
			error("Script tried to push invalid variable offset");
		_stack.push({ StackEntryType::Variable, index });
	}

	void pushString(uint32 offset) {
		if (offset >= _script._strings->size())
			error("Script tried to push invalid string offset");
		_stack.push({ StackEntryType::String, offset });
	}

	void pushInstruction(uint32 pc) {
		_stack.push({ StackEntryType::Instruction, pc });
	}

	StackEntry pop() {
		if (_stack.empty())
			error("Script tried to pop empty stack");
		return _stack.pop();
	}

	int32 popNumber() {
		auto entry = pop();
		if (entry._type != StackEntryType::Number)
			error("Script tried to pop, but top of stack is not a number");
		return entry._number;
	}

	int32 &popVariable() {
		auto entry = pop();
		if (entry._type != StackEntryType::Variable)
			error("Script tried to pop, but top of stack is not a variable");
		return _script._variables[entry._index];
	}

	const char *popString() {
		auto entry = pop();
		if (entry._type != StackEntryType::String)
			error("Script tried to pop, but top of stack is not a string");
		return _script._strings->data() + entry._index;
	}

	uint32 popInstruction() {
		auto entry = pop();
		if (entry._type != StackEntryType::Instruction)
			error("Script tried to pop but top of stack is not an instruction");
		return entry._index;
	}

	StackEntry getArg(uint argI) {
		if (_stack.size() < argI + 1)
			error("Script did not supply enough arguments for kernel call");
		return _stack[_stack.size() - 1 - argI];
	}

	int32 getNumberArg(uint argI) {
		auto entry = getArg(argI);
		if (entry._type != StackEntryType::Number)
			error("Expected number in argument %u for kernel call", argI);
		return entry._number;
	}

	const char *getStringArg(uint argI) {
		auto entry = getArg(argI);
		if (entry._type != StackEntryType::String)
			error("Expected string in argument %u for kernel call", argI);
		return _script._strings->data() + entry._index;
	}

	int32 getNumberOrStringArg(uint argI) {
		// Original inconsistency: sometimes a string is passed instead of a number
		// as it will be interpreted as a boolean we only care about == 0 / != 0
		auto entry = getArg(argI);
		if (entry._type != StackEntryType::Number && entry._type != StackEntryType::String)
			error("Expected number of string in argument %u for kernel call", argI);
		return entry._number;
	}

	MainCharacter &relatedCharacter() {
		if (process().character() == MainCharacterKind::None)
			error("Script tried to use character from non-character-related process");
		return g_engine->world().getMainCharacterByKind(process().character());
	}

	bool shouldSkipCutscene() {
		return process().character() != MainCharacterKind::None &&
			g_engine->player().activeCharacterKind() != process().character();
	}

	TaskReturn kernelCall(ScriptKernelTask task) {
		switch (task) {
		// sound/video
		case ScriptKernelTask::PlayVideo:
			g_engine->playVideo(getNumberArg(0));
			return TaskReturn::finish(0);
		case ScriptKernelTask::PlaySound:
			warning("STUB KERNEL CALL: PlaySound");
			return TaskReturn::finish(0);
		case ScriptKernelTask::PlayMusic:
			warning("STUB KERNEL CALL: PlayMusic");
			return TaskReturn::finish(0);
		case ScriptKernelTask::StopMusic:
			warning("STUB KERNEL CALL: StopMusic");
			return TaskReturn::finish(0);
		case ScriptKernelTask::WaitForMusicToEnd:
			warning("STUB KERNEL CALL: WaitForMusicToEnd");
			return TaskReturn::finish(0);

		// Misc / control flow
		case ScriptKernelTask::ShowCenterBottomText:
			warning("STUB KERNEL CALL: ShowCenterBottomText");
			return TaskReturn::finish(0);
		case ScriptKernelTask::Delay:
			return getNumberArg(0) <= 0
				? TaskReturn::finish(0)
				: TaskReturn::waitFor(delay((uint32)getNumberArg(0)));
		case ScriptKernelTask::HadNoMousePressFor:
			return TaskReturn::waitFor(new ScriptTimerTask(process(), getNumberArg(0)));
		case ScriptKernelTask::Fork:
			g_engine->scheduler().createProcess<ScriptTask>(process().character(), *this);
			return TaskReturn::finish(0); // 0 means this is the forking process
		case ScriptKernelTask::KillProcesses:
			warning("STUB KERNEL CALL: KillProcesses");
			return TaskReturn::finish(0);

		// player/world state changes
		case ScriptKernelTask::ChangeCharacter:
			warning("STUB KERNEL CALL: ChangeCharacter");
			return TaskReturn::finish(0);
		case ScriptKernelTask::ChangeRoom:
			if (strcmpi(getStringArg(0), "SALIR") == 0) {
				g_engine->quitGame();
				g_engine->player().changeRoom("SALIR", true);
			}
			else if (strcmpi(getStringArg(0), "MENUPRINCIPALINICIO") == 0)
				warning("STUB: change room to MenuPrincipalInicio special case");
			else {
				auto targetRoom = g_engine->world().getRoomByName(getStringArg(0));
				if (targetRoom == nullptr)
					error("Invalid room name: %s\n", getStringArg(0));
				if (process().isActiveForPlayer()) {
					g_engine->player().heldItem() = nullptr;
					if (g_engine->player().currentRoom() == &g_engine->world().inventory())
						g_engine->world().inventory().close();
					if (targetRoom == &g_engine->world().inventory())
						g_engine->world().inventory().open();
					else
						g_engine->player().changeRoom(targetRoom->name(), true);
					// TODO: Change music on kernel change room
				}
				g_engine->script().createProcess(process().character(), "ENTRAR_" + targetRoom->name(), ScriptFlags::AllowMissing);
			}
			return TaskReturn::finish(1);
		case ScriptKernelTask::ToggleRoomFloor:
			if (process().character() == MainCharacterKind::None) {
				if (g_engine->player().currentRoom() != nullptr)
					g_engine->player().currentRoom()->toggleActiveFloor();
			}
			else
				g_engine->world().getMainCharacterByKind(process().character()).room()->toggleActiveFloor();
			return TaskReturn::finish(1);
		case ScriptKernelTask::LerpWorldLodBias:
			warning("STUB KERNEL CALL: LerpWorldLodBias");
			return TaskReturn::finish(0);
		
		// object control / animation
		case ScriptKernelTask::On:
			g_engine->world().toggleObject(process().character(), getStringArg(0), true);
			return TaskReturn::finish(0);
		case ScriptKernelTask::Off:
			g_engine->world().toggleObject(process().character(), getStringArg(0), false);
			return TaskReturn::finish(0);
		case ScriptKernelTask::Animate: {
			auto object = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto graphicObject = dynamic_cast<GraphicObject *>(object);
			if (graphicObject == nullptr)
				error("Script tried to animate invalid graphic object %s", getStringArg(0));
			if (getNumberOrStringArg(1)) {
				graphicObject->toggle(true);
				graphicObject->graphic()->start(false);
				return TaskReturn::finish(1);
			}
			else
				return TaskReturn::waitFor(graphicObject->animate(process()));
		}

		// character control / animation
		case ScriptKernelTask::StopAndTurn: {
			auto object = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto character = dynamic_cast<WalkingCharacter *>(object);
			if (character == nullptr)
				error("Script tried to stop-and-turn unknown character");
			else
				character->stopWalking((Direction)getNumberArg(1));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::StopAndTurnMe: {
			relatedCharacter().stopWalking((Direction)getNumberArg(0));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::Go: {
			auto characterObject = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto character = dynamic_cast<WalkingCharacter *>(characterObject);
			if (character == nullptr)
				error("Script tried to make invalid character go: %s", getStringArg(0));
			auto targetObject = g_engine->world().getObjectByName(process().character(), getStringArg(1));
			auto target = dynamic_cast<PointObject *>(targetObject);
			if (target == nullptr)
				error("Script tried to make character go to invalid object %s", getStringArg(1));
			character->walkTo(target->position());

			if (getNumberArg(2) & 2)
				g_engine->camera().setFollow(nullptr);

			return (getNumberArg(2) & 1)
				? TaskReturn::finish(1)
				: TaskReturn::waitFor(character->waitForArrival(process()));
		}
		case ScriptKernelTask::Put: {
			auto characterObject = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto character = dynamic_cast<WalkingCharacter *>(characterObject);
			if (character == nullptr)
				error("Script tried to make invalid character go: %s", getStringArg(0));
			auto targetObject = g_engine->world().getObjectByName(process().character(), getStringArg(1));
			auto target = dynamic_cast<PointObject *>(targetObject);
			if (target == nullptr)
				error("Script tried to make character go to invalid object %s", getStringArg(1));
			character->setPosition(target->position());
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::ChangeCharacterRoom: {
			auto *character = dynamic_cast<Character *>(g_engine->world().globalRoom().getObjectByName(getStringArg(0)));
			if (character == nullptr)
				error("Invalid character name: %s", getStringArg(0));
			auto *targetRoom = g_engine->world().getRoomByName(getStringArg(1));
			if (targetRoom == nullptr)
				error("Invalid room name: %s", getStringArg(1));
			character->resetTalking();
			character->room() = targetRoom;
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::LerpCharacterLodBias:
			warning("STUB KERNEL CALL: LerpCharacterLodBias");
			return TaskReturn::finish(0);
		case ScriptKernelTask::AnimateCharacter:
			warning("STUB KERNEL CALL: AnimateCharacter");
			return TaskReturn::finish(0);
		case ScriptKernelTask::AnimateTalking:
			warning("STUB KERNEL CALL: AnimateTalking");
			return TaskReturn::finish(0);
		case ScriptKernelTask::SayText: {
			const char *characterName = getStringArg(0);
			int32 dialogId = getNumberArg(1);
			if (strncmp(characterName, "MENU_", 5) == 0) {
				g_engine->world().getMainCharacterByKind(process().character()).addDialogLine(dialogId);
				return TaskReturn::finish(1);
			}
			Character *_character = strcmp(characterName, "AMBOS") == 0
				? &g_engine->world().getMainCharacterByKind(process().character())
				: dynamic_cast<Character *>(g_engine->world().getObjectByName(characterName));
			if (_character == nullptr)
				error("Invalid character for sayText: %s", characterName);
			return TaskReturn::waitFor(_character->sayText(process(), dialogId));
		};
		case ScriptKernelTask::SetDialogLineReturn:
			g_engine->world().getMainCharacterByKind(process().character()).setLastDialogReturnValue(getNumberArg(0));
			return TaskReturn::finish(0);
		case ScriptKernelTask::DialogMenu:
			return TaskReturn::waitFor(g_engine->world().getMainCharacterByKind(process().character()).dialogMenu(process()));

		// Inventory control
		case ScriptKernelTask::Pickup:
			relatedCharacter().pickup(getStringArg(0), getNumberArg(1));
			return TaskReturn::finish(1);
		case ScriptKernelTask::CharacterPickup: {
			auto &character = g_engine->world().getMainCharacterByKind((MainCharacterKind)getNumberArg(1));
			character.pickup(getStringArg(0), getNumberArg(2));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::Drop:
			relatedCharacter().drop(getStringArg(0));
			return TaskReturn::finish(1);
		case ScriptKernelTask::CharacterDrop: {
			auto &character = g_engine->world().getMainCharacterByKind((MainCharacterKind)getNumberArg(1));
			character.drop(getStringArg(0));
			return TaskReturn::finish(1);
		}
		case ScriptKernelTask::ClearInventory:
			switch((MainCharacterKind)getNumberArg(0)) {
			case MainCharacterKind::Mortadelo: g_engine->world().mortadelo().clearInventory(); break;
			case MainCharacterKind::Filemon: g_engine->world().filemon().clearInventory(); break;
			default: error("Script attempted to clear inventory with invalid character kind"); break;
			}
			return TaskReturn::finish(1);

		// Camera tasks
		case ScriptKernelTask::SetMaxCamSpeedFactor:
			warning("STUB KERNEL CALL: SetMaxCamSpeedFactor");
			return TaskReturn::finish(0);
		case ScriptKernelTask::WaitCamStopping:
			return TaskReturn::waitFor(g_engine->camera().waitToStop(process()));
		case ScriptKernelTask::CamFollow:
			g_engine->camera().setFollow(
				&g_engine->world().getMainCharacterByKind((MainCharacterKind)getNumberArg(0)),
				getNumberArg(1) != 0);
			return TaskReturn::finish(1);
		case ScriptKernelTask::CamShake:
			warning("STUB KERNEL CALL: CamShake");
			return TaskReturn::finish(0);
		case ScriptKernelTask::LerpCamXY:
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				Vector2d(getNumberArg(0), getNumberArg(1)),
				getNumberArg(2), (EasingType)getNumberArg(3)));
		case ScriptKernelTask::LerpCamXYZ:
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				Vector3d(getNumberArg(0), getNumberArg(1), getNumberArg(2)),
				getNumberArg(3), (EasingType)getNumberArg(4)));
		case ScriptKernelTask::LerpCamZ:
			return TaskReturn::waitFor(g_engine->camera().lerpPosZ(process(),
				getNumberArg(0),
				getNumberArg(1), (EasingType)getNumberArg(2)));
		case ScriptKernelTask::LerpCamScale:
			return TaskReturn::waitFor(g_engine->camera().lerpScale(process(),
				getNumberArg(0) * 0.01f,
				getNumberArg(1), (EasingType)getNumberArg(2)));
		case ScriptKernelTask::LerpCamRotation:
			return TaskReturn::waitFor(g_engine->camera().lerpRotation(process(),
				getNumberArg(0),
				getNumberArg(1), (EasingType)getNumberArg(2)));
		case ScriptKernelTask::LerpCamToObjectKeepingZ: {
			if (!process().isActiveForPlayer())
				return TaskReturn::finish(0); // contrary to ...ResettingZ this one does not delay if not active
			auto object = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto pointObject = dynamic_cast<PointObject *>(object);
			if (pointObject == nullptr)
				error("Invalid target object for LerpCamToObjectKeepingZ: %s", getStringArg(0));
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				as2D(pointObject->position()),
				getNumberArg(1), EasingType::Linear));
		}
		case ScriptKernelTask::LerpCamToObjectResettingZ: {
			if (!process().isActiveForPlayer())
				return TaskReturn::waitFor(delay(getNumberArg(1)));
			auto object = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto pointObject = dynamic_cast<PointObject *>(object);
			if (pointObject == nullptr)
				error("Invalid target object for LerpCamToObjectResettingZ: %s", getStringArg(0));
			return TaskReturn::waitFor(g_engine->camera().lerpPos(process(),
				as3D(pointObject->position()),
				getNumberArg(1), (EasingType)getNumberArg(2)));
		}
		case ScriptKernelTask::LerpCamToObjectWithScale: {
			if (!process().isActiveForPlayer())
				return TaskReturn::waitFor(delay(getNumberArg(2)));
			auto object = g_engine->world().getObjectByName(process().character(), getStringArg(0));
			auto pointObject = dynamic_cast<PointObject *>(object);
			if (pointObject == nullptr)
				error("Invalid target object for LerpCamToObjectWithScale: %s", getStringArg(0));
			return TaskReturn::waitFor(g_engine->camera().lerpPosScale(process(),
				as3D(pointObject->position()), getNumberArg(1) * 0.01f,
				getNumberArg(2), (EasingType)getNumberArg(3), (EasingType)getNumberArg(4)));
		}

		// Fades
		case ScriptKernelTask::FadeType0:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				getNumberArg(0) * 0.01f, getNumberArg(1) * 0.01f,
				getNumberArg(2), (EasingType)getNumberArg(4), getNumberArg(3)));
		case ScriptKernelTask::FadeType1:
			return TaskReturn::waitFor(fade(process(), FadeType::ToWhite,
				getNumberArg(0) * 0.01f, getNumberArg(1) * 0.01f,
				getNumberArg(2), (EasingType)getNumberArg(4), getNumberArg(3)));
		case ScriptKernelTask::FadeIn:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				1.0f, 0.0f, getNumberArg(0), EasingType::Out, -5,
				PermanentFadeAction::UnsetFaded));
		case ScriptKernelTask::FadeOut:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				0.0f, 1.0f, getNumberArg(0), EasingType::Out, -5,
				PermanentFadeAction::SetFaded));
		case ScriptKernelTask::FadeIn2:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				0.0f, 1.0f, getNumberArg(0), (EasingType)getNumberArg(1), -5,
				PermanentFadeAction::UnsetFaded));
		case ScriptKernelTask::FadeOut2:
			return TaskReturn::waitFor(fade(process(), FadeType::ToBlack,
				1.0f, 0.0f, getNumberArg(0), (EasingType)getNumberArg(1), -5,
				PermanentFadeAction::SetFaded));

		// Unused and useless
		case ScriptKernelTask::SetActiveTextureSet:
			// Fortunately this seems to be unused.
			warning("STUB KERNEL CALL: SetActiveTextureSet");
			return TaskReturn::finish(0);
		case ScriptKernelTask::FadeType2:
			warning("STUB KERNEL CALL: FadeType2"); // Crossfade, unused from script
			return TaskReturn::finish(0);
		case ScriptKernelTask::Nop10:
		case ScriptKernelTask::Nop24:
		case ScriptKernelTask::Nop34:
			return TaskReturn::finish(0);
		default:
			error("Invalid kernel call");
			return TaskReturn::finish(0);
		}
	}

	Script &_script;
	Stack<StackEntry> _stack;
	String _name;
	uint32 _pc;
	bool _returnsFromKernelCall = false;
	bool _isFirstExecution = true;
	FakeLock _lock;
};

Process *Script::createProcess(MainCharacterKind character, const String &behavior, const String &action, ScriptFlags flags) {
	return createProcess(character, behavior + '/' + action, flags);
}

Process *Script::createProcess(MainCharacterKind character, const String &procedure, ScriptFlags flags) {
	uint32 offset;
	if (!_procedures.tryGetVal(procedure, offset)) {
		if (flags & ScriptFlags::AllowMissing)
			return nullptr;
		error("Unknown required procedure: %s", procedure.c_str());
	}
	FakeLock lock;
	if (!(flags & ScriptFlags::IsBackground))
		new (&lock) FakeLock(g_engine->player().semaphoreFor(character));
	Process *process = g_engine->scheduler().createProcess<ScriptTask>(character, procedure, offset, Common::move(lock));
	process->name() = procedure;
	return process;
}

void Script::updateCommonVariables() {
	if (g_engine->input().wasAnyMousePressed()) // yes, this variable is never reset by the engine
		variable("SeHaPulsadoRaton") = 1;

	if (variable("CalcularTiempoSinPulsarRaton")) {
		if (_scriptTimer == 0)
			_scriptTimer = g_system->getMillis();
	}
	else
		_scriptTimer = 0;

	variable("EstanAmbos") = g_engine->world().mortadelo().room() == g_engine->world().filemon().room();
	variable("textoson") = 1; // TODO: Add subtitle option
	variable("modored") = 1; // this is signalling whether a network connection is established
}

}

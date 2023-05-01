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

#include "common/memstream.h"
#include "topgun/topgun.h"

namespace TopGun {

Script::Script(TopGunEngine *engine) :
	_debugger(new ScriptDebugger(engine)),
	_engine(engine),
	_scene(nullptr),
	_nestedScriptCount(0),
	_localScope(0),
	_scriptResult(0),
	_reg3E3F(0),
	_mouseEventHandler(0),
	_pauseEventHandler(-1) {
	_systemVariables.resize(engine->getGameDesc()->_systemVarCount);
	memset(_keyListeners, 0, sizeof(_keyListeners));
}

Script::~Script() {
	for (auto procedure : _pluginProcedures)
		delete procedure;
}

void Script::prepareSceneChange() {
	_debugger->onScene(false);
	memset(_keyListeners, 0, sizeof(_keyListeners));
	_reg3E3F = 0;
}

void Script::runMessageQueue() {
}

void Script::runEntry() {
	_localVariables.clear();
	_stack.clear();
	_localScope = 0;
	for (auto procedure : _pluginProcedures)
		delete procedure;
	_pluginProcedures.clear();

	_scene = _engine->getScene();
	auto resFile = _engine->getResourceFile();
	_pluginProcedures.reserve(resFile->_pluginProcedures.size());
	for (size_t i = 0; i < resFile->_pluginProcedures.size(); i++) {
		auto pluginIndex = resFile->_pluginIndexPerProcedure[i];
		auto plugin = _engine->getLoadedPlugin(pluginIndex);
		auto procedure = plugin->getScriptProcedure(resFile->_pluginProcedures[i]);
		_pluginProcedures.push_back(procedure);
	}

	_debugger->onScene(true);

	debugCN(kTrace, kDebugScript, "Running scene entry %d\n", resFile->_entryId);
	_debugger->onCallStart(ScriptCallType::kRoot, resFile->_entryId);
	runScript(resFile->_entryId);
}

int32 Script::runMessage(uint32 index, uint32 localScopeSize, uint32 argCount, const int32 *args) {
	const auto prevResult = _scriptResult;
	_scriptResult = 0;

	if (debugChannelSet(kTrace, kDebugScript)) { // TODO: Move this into script debugger
		debugCN(kTrace, kDebugScript, "Running script %d", index);
		if (argCount > 0) {
			debugCN(kTrace, kDebugScript, " with");
			for (uint32 i = 0; i < argCount; i++)
				debugCN(kTrace, kDebugScript, " %d", args[i]);
		}
		debugCN(kTrace, kDebugScript, "\n");
	}

	_localScope += localScopeSize;
	setupLocalArguments(args, argCount);
	_debugger->onCallStart(ScriptCallType::kRoot, index, argCount, localScopeSize);
	runScript(index);
	_localScope -= localScopeSize;

	const auto newResult = _scriptResult;
	_scriptResult = prevResult;
	return newResult;
}

int32 Script::runMessage(uint32 index, int32 arg) {
	return runMessage(index, 0, 1, &arg);
}

int32 Script::runMessage(uint32 index) {
	return runMessage(index, 0, 0, nullptr);
}

void Script::runScript(uint32 index) {
	constexpr uint32 maxNesting = 30;
	if (++_nestedScriptCount > maxNesting)
		error("Too many nested scripts");

	auto scriptResource = _engine->loadResource<ScriptResource>(index);
	auto &scriptData = scriptResource->getData();
	Common::MemorySeekableReadWriteStream stream(scriptData.begin(), scriptData.size());
	runRoot(stream, index);

	_nestedScriptCount--;
	_debugger->onCallEnd();
}

void Script::runRoot(Common::MemorySeekableReadWriteStream &stream, uint32 index) {
	while (stream.pos() < stream.size() && !stream.err())
	{
		runSingleRootInstruction(stream, index);
		_debugger->onCallIncrement(stream.pos());
	}

	if (stream.err())
		error("Stream error during script execution");
}

int32 Script::runProcedure(uint32 procId, const int32 *args, uint32 argCount, uint32 scopeSize) {
	_localScope += scopeSize;
	_debugger->onCallStart(ScriptCallType::kProcedure, procId, argCount, scopeSize);

	const auto result = procId > _engine->getResourceFile()->_maxScrMsg
		? runPluginProcedure(procId, args, argCount)
		: runInternalProcedure(procId, args, argCount);

	_debugger->onCallEnd();
	_localScope -= scopeSize;
	return result;
}

int32 Script::runPluginProcedure(uint32 procId, const int32 *args, uint32 argCount) {
	const auto resFile = _engine->getResourceFile();
	const auto maxScrMsg = resFile->_maxScrMsg;
	debugCN(kVerbose, kDebugScript, "plugin procedure %d\n", procId);
	procId -= maxScrMsg;
	if (procId >= _pluginProcedures.size() || _pluginProcedures[procId] == nullptr)
		error("Unsupported plugin procedure id %d = (%s.%s)",
			procId + maxScrMsg,
			resFile->_plugins[resFile->_pluginIndexPerProcedure[procId]].c_str(),
			resFile->_pluginProcedures[procId].c_str());
	return (*_pluginProcedures[procId])(args, argCount);
}

int32 Script::readSint(Common::ReadStream &stream) {
	return _engine->getResourceFile()->_architecture == Architecture::kBits32
		? stream.readSint32LE()
		: stream.readSint16LE();
}

uint32 Script::readUint(Common::ReadStream &stream) {
	return _engine->getResourceFile()->_architecture == Architecture::kBits32
		? stream.readUint32LE()
		: stream.readUint16LE();
}

int32 Script::calcJumpOffset(uint32 nativeIntCount, uint32 additionalBytes) const {
	additionalBytes += 2; // the op code itself
	return _engine->getResourceFile()->_architecture == Architecture::kBits32
		? nativeIntCount * 4 + additionalBytes
		: nativeIntCount * 2 + additionalBytes;
}

int32 Script::stackTop() const {
	return _stack.back();
}

int32 Script::stackPop() {
	auto result = _stack.back();
	_stack.pop_back();
	return result;
}

void Script::stackPush(int32 value) {
	_stack.push_back(value);
}

int32 Script::evalValue(int32 valueOrIndex, bool isIndex) {
	const auto gameDesc = _engine->getGameDesc();
	if (!isIndex)
		return valueOrIndex;
	_debugger->onVariable(false, valueOrIndex);

	if (valueOrIndex < gameDesc->_globalVarCount)
		return _scene->getVariable(valueOrIndex);
	else if (valueOrIndex < gameDesc->_globalVarCount + gameDesc->_systemVarCount)
		return _systemVariables[valueOrIndex - gameDesc->_globalVarCount];
	else
	{
		const auto localIndex = _localScope + valueOrIndex - gameDesc->_globalVarCount - gameDesc->_systemVarCount;
		if (localIndex >= _localVariables.size())
			_localVariables.resize(localIndex + 1);
		return _localVariables[localIndex];
	}
}

void Script::setVariable(int32 index, int32 value) {
	const auto gameDesc = _engine->getGameDesc();
	_debugger->onVariable(true, index);
	if (index < gameDesc->_globalVarCount)
		_scene->setVariable(index, value);
	else if (index < gameDesc->_globalVarCount + gameDesc->_systemVarCount)
		_systemVariables[index - gameDesc->_globalVarCount] = value;
	else
	{
		const auto localIndex = _localScope + index - gameDesc->_globalVarCount - gameDesc->_systemVarCount;
		if (localIndex >= _localVariables.size())
			_localVariables.resize(localIndex + 1);
		_localVariables[localIndex] = value;
	}
}

void Script::setupLocalArguments(const int32 *args, uint32 argCount) {
	if (_localScope + argCount > _localVariables.size())
		_localVariables.resize(_localScope + argCount);

	for (uint32 i = 0; i < argCount; i++)
		_localVariables[_localScope + argCount - 1 - i] = args[i];
}

constexpr int32 kConstStrBit = 0x8000;

Common::String Script::getString(int32 index) {	
	const bool isConstString = index & kConstStrBit;
	index = index & (kConstStrBit - 1);
	return isConstString
		? _engine->getResourceFile()->getConstString(index)
		: _scene->getDynamicString(index - 1);
}

void Script::setString(int32 index, const Common::String &value) {
	const bool isConstString = index & kConstStrBit;
	index = index & (kConstStrBit - 1);
	if (isConstString)
		error("Attempted to modify const string %d", index);
	_scene->setDynamicString(index - 1, value);
}

void Script::onKeyDown(Common::KeyState keyState) {
	auto windowsKey = _engine->convertScummKeyToWindows(keyState.keycode);
	if (windowsKey < 0 || _keyListeners[windowsKey]._isDisabled)
		return;
	uint32 script;
	if (keyState.hasFlags(Common::KBD_SHIFT))
		script = _keyListeners[windowsKey]._scriptShift;
	else if (keyState.hasFlags(Common::KBD_CTRL))
		script = _keyListeners[windowsKey]._scriptControl;
	else if (keyState.hasFlags(Common::KBD_SHIFT | Common::KBD_CTRL))
		script = _keyListeners[windowsKey]._scriptShiftAndControl;
	else
		script = _keyListeners[windowsKey]._scriptUnmodified;
	if (script)
		runMessage(script, windowsKey);
}

void Script::onKeyUp(Common::KeyState keyState) {
	auto windowsKey = _engine->convertScummKeyToWindows(keyState.keycode);
	if (windowsKey >= 0 && _keyListeners[windowsKey]._scriptUp)
		runMessage(_keyListeners[windowsKey]._scriptUp, windowsKey);
}

void ScriptKeyListener::setDownScript(uint32 script, bool isForShift, bool isForControl) {
	_isDisabled = false;
	if (isForShift && isForControl)
		_scriptShiftAndControl = script;
	else if (isForShift)
		_scriptShift = script;
	else if (isForControl)
		_scriptControl = script;
	else
		_scriptUnmodified = script;
}

void Script::setKeyListener(int32 key, uint32 script, bool isForShift, bool isForControl) {
	if (key < 0)
		return;
	else if (key < kWindowsKeyCount)
		_keyListeners[key].setDownScript(script, isForShift, isForControl);
	else {
		for (int32 i = 0; i < kWindowsKeyCount; i++)
			_keyListeners[i].setDownScript(script, isForShift, isForControl);
	}
}

void Script::setKeyUpListener(int32 key, uint32 script) {
	if (key < 0)
		return;
	else if (key < kWindowsKeyCount) {
		_keyListeners[key]._scriptUp = script;
		_keyListeners[key]._isDisabled = false;
	}
	else {
		for (int32 i = 0; i < kWindowsKeyCount; i++) {
			_keyListeners[i]._scriptUp = script;
			_keyListeners[i]._isDisabled = false;
		}
	}
}

void Script::toggleKeyListener(int32 key, bool toggle) {
	if (key < 0)
		return;
	else if (key < kWindowsKeyCount)
		_keyListeners[key]._isDisabled = !toggle;
	else {
		for (int32 i = 0; i < kWindowsKeyCount; i++)
			_keyListeners[i]._isDisabled = !toggle;
	}
}

}

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
	_engine(engine),
	_scene(nullptr),
	_nestedScriptCount(0),
	_localScope(0),
	_scriptResult(0),
	_reg3E3F(0),
	_pauseEventHandler(-1) {
	_systemVariables.resize(engine->getGameDesc()->_systemVarCount);
}

Script::~Script() {
	for (auto procedure : _pluginProcedures)
		delete procedure;
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
	
	run(resFile->_entryId);
}

void Script::run(uint32 index) {
	debugCN(kTrace, kDebugScript, "Running %d\n", index);

	constexpr uint32 maxNesting = 30;
	if (++_nestedScriptCount > maxNesting)
		error("Too many nested scripts");

	auto scriptResource = _engine->loadResource<ScriptResource>(index);
	auto &scriptData = scriptResource->getData();
	Common::MemorySeekableReadWriteStream stream(scriptData.begin(), scriptData.size());
	runRoot(stream);

	_nestedScriptCount--;
}

void Script::runRoot(Common::MemorySeekableReadWriteStream &stream) {
	while (stream.pos() < stream.size() && !stream.err())
	{
		runSingleRootInstruction(stream);
	}

	if (stream.err())
		error("Stream error during script execution");
}

int32 Script::runProcedure(uint32 procId, const int32 *args, uint32 argCount) {
	debugCN(kVerbose, kDebugScript, "procedure %d\n", procId);
	return procId > _engine->getResourceFile()->_maxScrMsg
		? runPluginProcedure(procId, args, argCount)
		: runInternalProcedure(procId, args, argCount);
}

int32 Script::runPluginProcedure(uint32 procId, const int32 *args, uint32 argCount) {
	const auto resFile = _engine->getResourceFile();
	const auto maxScrMsg = resFile->_maxScrMsg;
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
	else if (valueOrIndex < gameDesc->_globalVarCount)
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

void Script::setupLocalArguments(int32 *args, uint32 argCount) {
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

}

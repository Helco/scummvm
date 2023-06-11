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

#include "topgun/plugins/TamaPlugin.h"
#include "topgun/topgun.h"

namespace TopGun {

TamaPlugin::TamaPlugin(TopGunEngine *engine) : IPlugin(engine) {
}

ScriptPluginProcedure *TamaPlugin::getScriptProcedure(const Common::String &name) {
	if (!name.compareToIgnoreCase("Tama7th_MakePersistent"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tama7thMakePersistent);
	else if (!name.compareToIgnoreCase("Volume_GetMidiIncrements"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::volumeGetIncrements);
	else if (!name.compareToIgnoreCase("Volume_GetWaveIncrements"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::volumeGetIncrements);
	else if (!name.compareToIgnoreCase("Dialog_SignalAttention"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Save_MakeFileName"))
		// this would create an absolute path from a relative one, no need for ScummVM
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("TamagoGetNumActive"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoGetNumActive);
	else if (!name.compareToIgnoreCase("Window_GenerateMouseMove"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::windowGenerateMouseMove);
	// TODO: Implement those stubs
	else if (!name.compareToIgnoreCase("Volume_GetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_GetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_SetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_SetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Window_ShowFullScreen"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Window_Restore"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Window_Show"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("TamagoGetNumScrap"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Dialog_SetLanguage"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("Internet_OpenURL"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::internetOpenURL);
	else
		return nullptr;
}

int32 TamaPlugin::tama7thMakePersistent(const int32 *args, uint32 argCount) {
	return argCount < 1 ? 0 : args[0];
}

int32 TamaPlugin::volumeGetIncrements(const int32 *args, uint32 argCount) {
	return 32;
}

int32 TamaPlugin::dialogSignalAttention(const int32 *args, uint32 argCount) {
	// if we really wanted to we could check if the game is in focus and if not beep
	return 0;
}

int32 TamaPlugin::stubReturnOne(const int32 *args, uint32 argCount) {
	return 1;
}

int32 TamaPlugin::internetOpenURL(const int32 *args, uint32 argCount) {
	if (argCount < 1)
		error("Invalid number of arguments for Internet_OpenURL");
	auto url = _engine->getScript()->getString(args[0]);
	warning("stub: Internet_OpenURL for %s", url.c_str());
	return 0;
}

constexpr const char *kSectionActive = "active";

int32 TamaPlugin::tamagoGetNumActive(const int32 *args, uint32 argCount) {
	auto &ini = _engine->getSavestate()->getINIFile();
	if (!ini.hasSection(kSectionActive))
		return 0;
	const auto section = ini.getKeys(kSectionActive);
	return section.size();
}

int32 TamaPlugin::windowGenerateMouseMove(const int32 *args, uint32 argCount) {
	Common::Event event;
	event.type = Common::EVENT_MOUSEMOVE;
	// TODO: We should probably set the current mouse pos in this event
	event.relMouse = Common::Point(0, 0);
	_engine->getEventManager()->pushEvent(event);
	return 1;
}

}

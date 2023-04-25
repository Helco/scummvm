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
	// TODO: Implement those stubs
	else if (!name.compareToIgnoreCase("Volume_GetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_GetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
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

}

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

#ifndef TOPGUN_TAMAPLUGIN_H
#define TOPGUN_TAMAPLUGIN_H

#include "common/formats/winexe.h"
#include "topgun/script/IPlugin.h"

namespace TopGun {

class TamaPlugin : public IPlugin {
public:
	TamaPlugin(TopGunEngine *engine);
	virtual ~TamaPlugin() = default;

	virtual ScriptPluginProcedure *getScriptProcedure(const Common::String &name) override;

private:
	int32 tama7thMakePersistent(const int *args, uint32 argCount);
	int32 volumeGetIncrements(const int *args, uint32 argCount);
	int32 dialogPrompt(const int *args, uint32 argCount);
	int32 dialogSignalAttention(const int *args, uint32 argCount);
	int32 stubReturnOne(const int *args, uint32 argCount);
	int32 internetOpenURL(const int *args, uint32 argCount);
	int32 tamagoGetNumActive(const int *args, uint32 argCount);
	int32 windowGenerateMouseMove(const int *args, uint32 argCount);

	Common::ScopedPtr<Common::WinResources> _tamaResources;
};

}

#endif // TOPGUN_TAMAPLUGIN_H

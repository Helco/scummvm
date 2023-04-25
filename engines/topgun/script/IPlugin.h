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

#ifndef TOPGUN_IPLUGIN_H
#define TOPGUN_IPLUGIN_H

#include "common/hashmap.h"
#include "common/func.h"

namespace TopGun {
class TopGunEngine;

typedef Common::Functor2<const int32 *, uint32, int32> ScriptPluginProcedure;
template<class T>
using ScriptPluginProcedureMem = Common::Functor2Mem<const int32 *, uint32, int32, T>;

class IPlugin {
public:
	IPlugin(TopGunEngine *engine);
	virtual ~IPlugin() = default;

	virtual ScriptPluginProcedure *getScriptProcedure(const Common::String &name) = 0;

protected:
	TopGunEngine *_engine;
};

}

#endif // TOPGUN_IPLUGIN_H

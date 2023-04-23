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

#ifndef TOPGUN_SCENE_H
#define TOPGUN_SCENE_H

#include "common/system.h"

namespace TopGun {
class TopGunEngine;

class Scene {
public:
	Scene(TopGunEngine *engine, const Common::String &name);

	const Common::String &getName() const;
	int32 getVariable(int32 index) const;
	void setVariable(int32 index, int32 value);
	Common::String &getDynamicString(int32 index);
	void setDynamicString(int32 index, const Common::String &str);

private:
	Common::String _name;
	Common::Array<int32> _variables;
	Common::Array<Common::String> _dynamicStrings;
};

}

#endif // TOPGUN_SCENE_H

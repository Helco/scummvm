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

Scene::Scene(TopGunEngine *engine, const Common::String &name) : _name(name) {
	auto resFile = engine->getResourceFile();
	_variables.resize(engine->getGameDesc()->_varTableSize);
	for (const auto kv : resFile->_variables)
		_variables[kv._key] = kv._value;

	_dynamicStrings.resize(resFile->_dynamicStringCount);
}

const Common::String &Scene::getName() const {
	return _name;
}

int32 Scene::getVariable(int32 index) const {
	return _variables[index];
}

void Scene::setVariable(int32 index, int32 value) {
	_variables[index] = value;
}

Common::String &Scene::getDynamicString(int32 index) {
	return _dynamicStrings[index];
}

void Scene::setDynamicString(int32 index, const Common::String &string) {
	_dynamicStrings[index] = string;
}

}

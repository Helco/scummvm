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

#include "topgun/Savestate.h"

namespace TopGun {
static const char *kRegistrySection = "windows-registry";
static const char *kRegistryDefaultSubKey = "7thlevel";

Savestate::Savestate() : _iniFile(new Common::INIFile()) {
}

int32 Savestate::getRegistryNumber(
	int32 key,
	const char *subKey,
	const char *subSubKey,
	const char *valueName) {
	auto iniKey = createRegistryKey(key, subKey, subSubKey, valueName);
	Common::String iniValue;
	return _iniFile->getKey(iniKey, kRegistrySection, iniValue)
		? atoi(iniValue.c_str())
		: 0;
}

void Savestate::setRegistryNumber(
	int32 key,
	const char *subKey,
	const char *subSubKey,
	const char *valueName,
	int32 value) {
	auto iniKey = createRegistryKey(key, subKey, subSubKey, valueName);
	auto iniValue = Common::String::format("%d", value);
	_iniFile->setKey(iniKey, kRegistrySection, iniValue);
}

Common::String Savestate::getRegistryString(
	int32 key,
	const char *subKey,
	const char *subSubKey,
	const char *valueName) {
	auto iniKey = createRegistryKey(key, subKey, subSubKey, valueName);
	Common::String iniValue;
	return _iniFile->getKey(iniKey, kRegistrySection, iniValue)
		? iniValue
		: "";
}

void Savestate::setRegistryString(
	int32 key,
	const char *subKey,
	const char *subSubKey,
	const char *valueName,
	const char *value) {
	auto iniKey = createRegistryKey(key, subKey, subSubKey, valueName);
	_iniFile->setKey(iniKey, kRegistrySection, value);
}

void Savestate::deleteRegistryValue(
	int32 key,
	const char *subKey,
	const char *subSubKey,
	const char *valueName) {
	auto iniKey = createRegistryKey(key, subKey, subSubKey, valueName);
	_iniFile->removeKey(iniKey, kRegistrySection);
}

Common::String Savestate::createRegistryKey(
	int32 key,
	const char *subKey,
	const char *subSubKey,
	const char *valueName) {
	if (subKey == nullptr)
		subKey = kRegistryDefaultSubKey;
	auto iniKey = Common::String::format("%d-%s-%s-%s", key, subKey, subSubKey, valueName);
	for (size_t i = 0; i < iniKey.size(); i++) {
		if (!Common::isAlnum(iniKey[i]) && iniKey[i] != '-' && iniKey[i] != '_')
			iniKey.setChar('_', i);
	}
	return iniKey;
}

}

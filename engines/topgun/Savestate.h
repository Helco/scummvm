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

#ifndef TOPGUN_SAVESTATE_H
#define TOPGUN_SAVESTATE_H

#include "common/ptr.h"
#include "common/formats/ini-file.h"

using Common::ScopedPtr;

namespace TopGun {

/* There is no standardized savestate file (or location) for TopGun games.
 * Instead the various script languages have instructions to read/write
 * INI or Windows registry values.
 * Some games additionally have extra file IO plugins which makes this
 * even more complicated.
 *
 * But: if we cannot use the original savestates anyway, we also do not
 * have to care about the encryption that would be used.
 */

class Savestate {
public:
	static constexpr int32 kRegistryLocalMachineKey = 0x80000002;

	Savestate();

	int32 getRegistryNumber(
		int32 key,
		const char *subKey,
		const char *subSubKey,
		const char *valueName);

	void setRegistryNumber(
		int32 key,
		const char *subKey,
		const char *subSubKey,
		const char *valueName,
		int32 value);

	Common::String getRegistryString(
		int32 key,
		const char *subKey,
		const char *subSubKey,
		const char *valueName);

	void setRegistryString(
		int32 key,
		const char *subKey,
		const char *subSubKey,
		const char *valueName,
		const char *value);

	void deleteRegistryValue(
		int32 key,
		const char *subKey,
		const char *subSubKey,
		const char *valueName);

	inline Common::INIFile &getINIFile() {
		return *_iniFile;
	}

private:
	static Common::String createRegistryKey(
		int32 key,
		const char *subKey,
		const char *subSubKey,
		const char *valueName);

private:
	ScopedPtr<Common::INIFile> _iniFile;
};

}

#endif // TOPGUN_SAVESTATE_H

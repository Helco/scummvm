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

namespace TopGun {

const PlainGameDescriptor topgunGames[] = {
	{ "tama", "Tamagotchi CD-ROM" },
	{ 0, 0 }
};

const TopGunGameDescription gameDescriptions[] = {
	{
		{
			"tama",
			nullptr,
			AD_ENTRY1s("tama.bin", "903ca3bedb95a703a1b67d069fe62977", 180505),
			Common::EN_ANY,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NONE)
		},
		5001, // globalVarCount
		117 // systemVarCount
	},

	{
		AD_TABLE_END_MARKER,
		0,
		0
	}
};

} // End of namespace Topgun

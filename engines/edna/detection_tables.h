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

namespace Edna {

const PlainGameDescriptor ednaGames[] = {
	{ "edna", "Edna & Harvey: The Breakout" },
	{ 0, 0 }
};

const ADGameDescription gameDescriptions[] = {
	{
		// Steam 1.3.1
		"edna",
		"Steam v1.3.1",
		AD_ENTRY2s(
			"script/de/skript.csv", "c5fa29ec96154b1dc712563ceaf361bc", 3211243,
			"script/en/skript.csv", "3a244ac8307b662ba170e20b0a31a29c", 3145984
		),
		Common::DE_DEU,
		Common::kPlatformWindows,
		ADGF_UNSTABLE | ADGF_ADDENGLISH,
		GUIO1(GUIO_NONE)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Edna

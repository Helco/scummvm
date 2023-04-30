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

#include "topgun.h"

using namespace Common;

namespace TopGun {

Common::KeyCode TopGunEngine::convertWindowsKeyToScumm(int32 vk) {
	if (_windowsToScummKey[0x1B] != KEYCODE_ESCAPE) {
		for (int i = 0; i < KEYCODE_LAST; i++) {
			int32 mapped = convertScummKeyToWindows((KeyCode)i);
			if (mapped >= 0)
				_windowsToScummKey[mapped] = (KeyCode)i;
		}
	}
	if (vk < 0 || vk >= kWindowsKeyCount)
		return KEYCODE_INVALID;
	return _windowsToScummKey[vk];
}

int32 TopGunEngine::convertScummKeyToWindows(KeyCode code) {
	// values from https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
	// not complete, but works for now in the tested game(s)

	switch (code) {
	case KEYCODE_BACKSPACE: return 0x08;
	case KEYCODE_TAB: return 0x09;
	case KEYCODE_CLEAR: return 0x0C;
	case KEYCODE_RETURN: return 0x0D;
	case KEYCODE_PAUSE: return 0x13;
	case KEYCODE_ESCAPE: return 0x1B;
	case KEYCODE_SPACE: return 0x20;
	case KEYCODE_0: return 0x30;
	case KEYCODE_1: return 0x31;
	case KEYCODE_2: return 0x32;
	case KEYCODE_3: return 0x33;
	case KEYCODE_4: return 0x34;
	case KEYCODE_5: return 0x35;
	case KEYCODE_6: return 0x36;
	case KEYCODE_7: return 0x37;
	case KEYCODE_8: return 0x38;
	case KEYCODE_9: return 0x39;

	case KEYCODE_a: return 0x41;
	case KEYCODE_b: return 0x42;
	case KEYCODE_c: return 0x43;
	case KEYCODE_d: return 0x44;
	case KEYCODE_e: return 0x45;
	case KEYCODE_f: return 0x46;
	case KEYCODE_g: return 0x47;
	case KEYCODE_h: return 0x48;
	case KEYCODE_i: return 0x49;
	case KEYCODE_j: return 0x4A;
	case KEYCODE_k: return 0x4B;
	case KEYCODE_l: return 0x4C;
	case KEYCODE_m: return 0x4D;
	case KEYCODE_n: return 0x4E;
	case KEYCODE_o: return 0x4F;
	case KEYCODE_p: return 0x50;
	case KEYCODE_q: return 0x51;
	case KEYCODE_r: return 0x52;
	case KEYCODE_s: return 0x53;
	case KEYCODE_t: return 0x54;
	case KEYCODE_u: return 0x55;
	case KEYCODE_v: return 0x56;
	case KEYCODE_w: return 0x57;
	case KEYCODE_x: return 0x58;
	case KEYCODE_y: return 0x59;
	case KEYCODE_z: return 0x5A;
	case KEYCODE_DELETE: return 0x2E;

	case KEYCODE_KP0: return 0x60;
	case KEYCODE_KP1: return 0x61;
	case KEYCODE_KP2: return 0x62;
	case KEYCODE_KP3: return 0x63;
	case KEYCODE_KP4: return 0x64;
	case KEYCODE_KP5: return 0x65;
	case KEYCODE_KP6: return 0x66;
	case KEYCODE_KP7: return 0x67;
	case KEYCODE_KP8: return 0x68;
	case KEYCODE_KP9: return 0x69;
	case KEYCODE_KP_PERIOD: return 0x6C;
	case KEYCODE_KP_DIVIDE: return 0x6F;
	case KEYCODE_KP_MULTIPLY: return 0x6A;
	case KEYCODE_KP_MINUS: return 0x6D;
	case KEYCODE_KP_PLUS: return 0x6B;

	case KEYCODE_UP: return 0x26;
	case KEYCODE_DOWN: return 0x28;
	case KEYCODE_RIGHT: return 0x27;
	case KEYCODE_LEFT: return 0x25;
	case KEYCODE_INSERT: return 0x2D;
	case KEYCODE_HOME: return 0x24;
	case KEYCODE_END: return 0x23;
	case KEYCODE_PAGEUP: return 0x21;
	case KEYCODE_PAGEDOWN: return 0x22;

	case KEYCODE_F1: return 0x70;
	case KEYCODE_F2: return 0x71;
	case KEYCODE_F3: return 0x72;
	case KEYCODE_F4: return 0x73;
	case KEYCODE_F5: return 0x74;
	case KEYCODE_F6: return 0x75;
	case KEYCODE_F7: return 0x76;
	case KEYCODE_F8: return 0x77;
	case KEYCODE_F9: return 0x78;
	case KEYCODE_F10: return 0x79;
	case KEYCODE_F11: return 0x7A;
	case KEYCODE_F12: return 0x7B;
	case KEYCODE_F13: return 0x7C;
	case KEYCODE_F14: return 0x7D;
	case KEYCODE_F15: return 0x7E;
	case KEYCODE_F16: return 0x7F;
	case KEYCODE_F17: return 0x80;
	case KEYCODE_F18: return 0x81;

	case KEYCODE_NUMLOCK: return 0x90;
	case KEYCODE_CAPSLOCK: return 0x14;
	case KEYCODE_SCROLLOCK: return 0x91;
	case KEYCODE_RSHIFT: return 0xA1;
	case KEYCODE_LSHIFT: return 0xA0;
	case KEYCODE_RCTRL: return 0xA3;
	case KEYCODE_LCTRL: return 0xA2;
	case KEYCODE_RALT: return 0xA5;
	case KEYCODE_LALT: return 0xA4;
	case KEYCODE_LSUPER: return 0x5B;
	case KEYCODE_RSUPER: return 0x4C;

	case KEYCODE_HELP: return 0x2F;
	case KEYCODE_PRINT: return 0x2A;
	case KEYCODE_BREAK: return 0x03;
	case KEYCODE_SLEEP: return 0x5F;
	case KEYCODE_MUTE: return 0xAD;
	case KEYCODE_VOLUMEUP: return 0xAF;
	case KEYCODE_VOLUMEDOWN: return 0xAE;
	case KEYCODE_MAIL: return 0xB4;

	case KEYCODE_AUDIONEXT: return 0xB0;
	case KEYCODE_AUDIOPREV: return 0xB1;
	case KEYCODE_AUDIOSTOP: return 0xB2;
	case KEYCODE_AUDIOPLAYPAUSE: return 0xB3;
	default: return -1;
	}
}

}

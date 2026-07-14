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

#ifndef EDNA_SPRITE_H
#define EDNA_SPRITE_H

#include "edna/graphics.h"

namespace Edna {

class Sprite {
public:
	virtual ~Sprite();

	inline bool &active() { return _active; }
	inline Common::Rect &bounds() { return _bounds; }

	virtual void update();
	virtual void render();

	virtual void setTexture(Common::SharedPtr<ITexture> texture);
	virtual bool checkClick(Common::Point screenPos) const;

private:
	bool _active = true;
	Common::Rect _bounds;
	Common::SharedPtr<ITexture> _texture;
};

}

#endif // EDNA_SPRITE_H

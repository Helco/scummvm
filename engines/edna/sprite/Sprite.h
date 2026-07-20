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
#include "edna/util.h"

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
	friend class AnimatedSprite;
	bool _active = true;
	Common::Rect _bounds;
	Common::SharedPtr<ITexture> _texture;
};

struct AnimationRange {
	uint32 _startFrame = 0, _endFrame = 0, _delay = 0;
};

class AnimatedSprite : public Sprite {
public:
	inline uint32 &curFrame() { return _curFrame; }

	void update() override;
	void setTexture(Common::SharedPtr<ITexture> texture) override;
	void setTextures(Common::Span<Common::SharedPtr<ITexture>> textures);
	void setTextures(Common::Array<Common::SharedPtr<ITexture>> &&textures);
	void setAnimation(AnimationRange animation);
	void stopAnimation();

private:
	void resetTextures();

	Common::Array<Common::SharedPtr<ITexture>> _textures;
	uint32 _curFrame = 0;
	AnimationRange _animation;
	bool _loop = false;
	Timer _timer;
};

}

#endif // EDNA_SPRITE_H

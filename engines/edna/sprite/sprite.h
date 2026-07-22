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

class Animation;

class Sprite {
public:
	virtual ~Sprite();

	inline uint32 &id() { return _id; }
	inline bool active() const { return _active; }
	inline bool &immutable() { return _immutable; } ///< An immutable sprite is not deleted if inactive
	inline Common::Point &pos() { return _pos; }
	inline Common::Point pos() const { return _pos; }
	inline Common::Point size() const { return _size; }
	inline Common::Rect bounds() const { return { _pos, _pos + _size }; }

	virtual void toggle(bool isActive);
	virtual void update();
	virtual void render();
	virtual void debugPrint();

	void setTexture(const char *fileName); ///< shortcut for loading textures
	virtual void setTexture(TexturePtr texture);
	virtual bool checkClick(Common::Point screenPos) const;

private:
	friend class AnimatedSprite;
	bool _active = true, _immutable = false;
	Common::Point _pos, _size;
	TexturePtr _texture;
	uint32 _id = 0;
};

class AnimatedSprite : public virtual Sprite {
public:
	inline uint32 &curFrame() { return _curFrame; }
	inline bool isAnimating() const { return _timer.active(); }

	void update() override;
	void debugPrint() override;
	using Sprite::setTexture;
	void setTexture(TexturePtr texture) override;
	void setTextures(std::initializer_list<const char *> fileNames);
	void setTextures(TextureSpan textures);
	void setTextures(TextureArray &&textures);
	void setAnimation(AnimationRange animation);
	void setAnimation(const Animation& animation);
	void startAnimation();
	void stopAnimation();
	void setFrame(uint32 index);

private:
	void resetTextures();

	TextureArray _textures;
	uint32 _curFrame = 0;
	AnimationRange _animation;
	Timer _timer;
};

}

#endif // EDNA_SPRITE_H

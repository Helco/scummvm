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

#include "edna/edna.h"
#include "edna/sprite/Animation.h"
#include "edna/sprite/Sprite.h"

using namespace Common;

namespace Edna {

Sprite::~Sprite() { }

void Sprite::update() { }

void Sprite::render() {
	if (_active && _texture != nullptr)
		g_engine->renderer().sprite(_texture.get(), _bounds.origin(), Point(_bounds.width(), _bounds.height()));
}

void Sprite::setTexture(TexturePtr texture) {
	_texture = texture;
	if (texture == nullptr) {
		_bounds.setWidth(0);
		_bounds.setHeight(0);
	} else {
		_bounds.setWidth(texture->size().x);
		_bounds.setHeight(texture->size().y);
	}
}

bool Sprite::checkClick(Point screenPos) const {
	return
		_active &&
		_texture != nullptr &&
		_bounds.contains(screenPos) &&
		_texture->alphaCheck(screenPos - _bounds.origin());
}

void AnimatedSprite::update() {
	Sprite::update();
	if (_active && _timer.update()) {
		if (_curFrame < _animation._endFrame)
			_curFrame++;
		else if (_animation._loop)
			_curFrame = _animation._startFrame;
		else
			_timer.toggle(false);
		_texture = _textures[_curFrame]; // for Sprite to render
	}
}

void AnimatedSprite::setTexture(TexturePtr texture) {
	setTextures(TextureSpan { &texture, 1 });
}

void AnimatedSprite::setTextures(TextureSpan textures) {
	_textures.resize(textures.size());
	copy(textures.begin(), textures.end(), _textures.data());
	resetTextures();
}

void AnimatedSprite::setTextures(TextureArray &&textures) {
	_textures = move(textures);
	resetTextures();
}

void AnimatedSprite::setAnimation(const Animation &animation) {
	setTextures(animation.textures());
	setAnimation(animation.range());
}

void AnimatedSprite::resetTextures() {
	assert(_textures.size() > 0);
	assert(find(_textures.begin(), _textures.end(), nullptr) == _textures.end());
	stopAnimation();
}

void AnimatedSprite::setAnimation(AnimationRange animation) {
	assert(animation._startFrame < _textures.size());
	assert(animation._endFrame < _textures.size());
	assert(animation._startFrame <= animation._endFrame);
	_animation = animation;
	_curFrame = animation._startFrame;
	_texture = _textures[_curFrame];
	_timer.delay() = animation._delay;
	_timer.toggle(false);
}

void AnimatedSprite::stopAnimation() {
	_animation = {};
	_timer.toggle(false);
	_curFrame = _animation._startFrame;
	_texture = _textures.empty() ? nullptr : _textures[_curFrame];
}

}

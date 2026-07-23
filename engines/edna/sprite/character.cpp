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
#include "edna/sprite/character.h"

using namespace Common;

namespace Edna {

Character::Character(
	Point startPos,
	CharAnimSetId charAnimSetId,
	float hSpeed, float vSpeed,
	float baseYAtZeroScale, float baseYAtFullScale)
	: _charAnimSet(charAnimSetId)
	, _hSpeed(hSpeed), _vSpeed(vSpeed)
	, _baseYAtZeroScale(baseYAtZeroScale), _baseYAtFullScale(baseYAtFullScale) {

	pos() = startPos;
	setTextures(_charAnimSet.textures());
	initScaling();
}

void Character::update() {
	AnimatedSprite::update();

	updateTalking();

	if (_stateTimer.update()) {
		_stateTimer.toggle(false);
		setState(kWaiting);
	}

	updateScaling();
}

void Character::wait(uint32 duration) {
	assert(_state == kWaiting); // this might not hold true
	_state = kActing;
	_stateTimer.delay() = duration;
	_stateTimer.toggle(true);
}

void Character::lookIn(Direction direction) {
	assert(_state == kWaiting);
	_direction = direction;
	setState(kWaiting);
}

void Character::say(const char *text, const char *soundFile) {
	sayOrThink(text, soundFile, kTalking);
}

void Character::think(const char *text, const char *soundFile) {
	sayOrThink(text, soundFile, kThinking);
}

void Character::sayOrThink(const char *text, const char *soundFile, State newState) {
	assert(text != nullptr);
	setState(newState);

	auto &config = g_engine->config();
	if (soundFile != nullptr && *soundFile && config.speech()) {
		// TODO: Play speech sound
	}

	if (config.subtitles() || !config.speech()) {
		// TODO: Show speech text
	}

	if (_talkSound == Audio::SoundHandle()) {
		uint32 duration = strlen(text);
		duration = ((duration < 15) ? 2 : 1) * duration * (255 - config.subtitleSpeed());
		_talkEndTime = g_engine->getMillis() + duration;
	}
}

void Character::updateTalking() {
	if ((_state != kTalking && _state != kThinking) || // not talking at all
		(_talkSound != Audio::SoundHandle() && g_system->getMixer()->isSoundHandleActive(_talkSound)) || // still speaking
		_talkEndTime > g_engine->getMillis()) // still whispering
		return;
	shutUp();
}

void Character::shutUp() {
	if (_state != kActing && _state != kTalking && _state != kThinking)
		return;

	if (_talkText != nullptr)
		_talkText->toggle(false);
	_talkText = nullptr;
	g_system->getMixer()->stopHandle(_talkSound);
	_talkSound = {};
	setState(kWaiting);
}

void Character::setState(State state) {
	_state = state;
	setAnimation(state);
}

void Character::setAnimation() {
	setAnimation(_actionMode);
}

void Character::setAnimation(ActionModeId actionMode) {
	_actionMode = actionMode;
	auto animation = _charAnimSet.get(actionMode, _direction);
	if (textureCount() != _charAnimSet.textures().size())
		setTextures(_charAnimSet.textures()); // we loaded an animation on-demand
	setAnimation(animation);
}

void Character::initScaling() {
	// TODO: This might differ between NPCs and Player -_-
	const Point size = this->size();
	_scaleFactor = 1.0f / (_baseYAtFullScale - _baseYAtZeroScale - size.y);

	const Point baseOffset(size.x / 2, size.y);
	float initScale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		initScale = (pos().y - _baseYAtZeroScale) / (_baseYAtFullScale - _baseYAtZeroScale);
	pos() -= Point(size.x / 2, size.y) * initScale;
	updateScaling();
}

void Character::updateScaling() {
	float scale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		scale = MAX(0.001f, ABS(pos().y - _baseYAtZeroScale) * _scaleFactor);
	size() = _baseSize * scale;
}

const char *Character::stateToString() const {
	switch (_state) {
	case kWalking:
		return "walking";
	case kTalking:
		return "talking";
	case kThinking:
		return "thinking";
	case kWaiting:
		return "waiting";
	case kActing:
		return "acting";
	default:
		return "unknown";
	}
}

}

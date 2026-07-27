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
#include "edna/game/game.h"
#include "edna/sprite/character.h"
#include "edna/sprite/text.h"

#include "math/vector2d.h"

using namespace Common;
using namespace Math;

namespace Edna {

Character::Character(
	Game &game,
	Point startPos,
	CharAnimSetId charAnimSetId,
	float hSpeed, float vSpeed,
	float baseYAtZeroScale, float baseYAtFullScale)
	: _game(game)
	, _charAnimSet(charAnimSetId)
	, _hSpeed(hSpeed), _vSpeed(vSpeed)
	, _baseYAtZeroScale(baseYAtZeroScale), _baseYAtFullScale(baseYAtFullScale) {

	pos() = startPos;
	setTextures(_charAnimSet.textures());
	initScaling();
	setState(kWaiting); // also sets the animation
}

void Character::update() {
	AnimatedSprite::update();

	updateTalking();
	updateWalking();

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
	if (soundFile != nullptr && *soundFile && config.speech())
		_talkSound = g_engine->playSpeech(soundFile);

	if (*text && (config.subtitles() || !config.speech())) {
		_talkText = new Text(
			Point(basePosX(), pos().y),
			newState == State::kTalking ? _talkFont : _thinkFont,
			text,
			(TextFlags)(kTextWrapLines | kTextMoveIntoScreen));
		_game.texts().add(_talkText, DisposeAfterUse::YES);
	}

	if (_talkSound == Audio::SoundHandle()) {
		uint32 duration = strlen(text);
		_talkTimeLeft = ((duration < 15) ? 2 : 1) * duration * (255 - config.subtitleSpeed());
	}
}

void Character::updateTalking() {
	if ((_state != kTalking && _state != kThinking) || // not talking at all
		(_talkSound != Audio::SoundHandle() && g_system->getMixer()->isSoundHandleActive(_talkSound))) // still speaking
		return;
	if (_talkTimeLeft > 0) { // still speaking without sound
		if (_talkTimeLeft > g_engine->getElapsed()) {
			_talkTimeLeft -= g_engine->getElapsed();
			return;
		}
		_talkTimeLeft = 0;
	}
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

void Character::walkTo(Point walkTo, Direction standbyDirection) {
	_standbyDirection = standbyDirection;
	_walkPoints.clear();
	_walkPoints.emplace_back(walkTo);
	nextWalkPoint();
}

void Character::updateWalking() {
	if (_state != kWalking)
		return;

	Point delta = _walkTarget - pos();
	Vector2d move = Vector2d(delta.x, delta.y).getNormalized() * _cSpeed * g_engine->getElapsed();
	const auto absmin = [](int16 delta, float move) -> int16 {
		int16 moveI = (int16)(move + 0.5f);
		return ABS(delta) < ABS(moveI) ? delta : moveI;
	};
	pos().x += absmin(delta.x, move.getX());
	pos().y += absmin(delta.y, move.getY());

	if (pos() == _walkTarget)
		nextWalkPoint();
}

void Character::nextWalkPoint() {
	if (_walkPoints.empty()) {
		if (_standbyDirection != Direction::None)
			_direction = _standbyDirection;
		setState(kWaiting);
		return;
	}

	_walkTarget = scaleBasePosToSpritePos(_walkPoints.back());
	Point signedDelta = _walkPoints.back() - Point(basePosX(), basePosY());
	Point delta(ABS(signedDelta.x), ABS(signedDelta.y));
	_cSpeed = (_hSpeed * delta.x + _vSpeed * delta.y) / (delta.x + delta.y);
	_direction = delta.x > delta.y
		? (signedDelta.x < 0 ? Direction::Left : Direction::Right)
		: (signedDelta.y < 0 ? Direction::Up : Direction::Down);
	setState(kWalking);
	_walkPoints.pop_back();
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
	_baseOffset = Point(size().x / 2, size().y);
	_scaleFactor = 1.0f / (_baseYAtFullScale - _baseYAtZeroScale - size().y);
	pos() = scaleBasePosToSpritePos(pos());
	updateScaling();
}

void Character::updateScaling() {
	float scale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		scale = MAX(0.001f, ABS((pos().y - _baseYAtZeroScale) * _scaleFactor));
	size() = curFrameSize() * scale;
}

Point Character::scaleBasePosToSpritePos(Point basePos) const {
	float scale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		scale = (basePos.y - _baseYAtZeroScale) / (_baseYAtFullScale - _baseYAtZeroScale);
	return basePos - _baseOffset * scale;
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

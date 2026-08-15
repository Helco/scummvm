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

#include "edna/console.h"
#include "edna/edna.h"
#include "edna/game/game.h"
#include "edna/graphics.h"
#include "edna/pathfinder.h"
#include "edna/sprite/character.h"
#include "edna/sprite/text.h"

#include "gui/debugger.h"

using namespace Common;
using namespace Math;

namespace Edna {

Character::Character(
	Game &game,
	Point startPos,
	CharAnimSetId charAnimSetId,
	float hSpeed, float vSpeed,
	float baseYAtZeroScale, float baseYAtFullScale,
	Point baseLineStart, Point baseLineEnd)
	: GameObject(baseLineStart, baseLineEnd)
	, _game(game)
	, _charAnimSet(charAnimSetId)
	, _hSpeed(hSpeed), _vSpeed(vSpeed)
	, _baseYAtZeroScale(baseYAtZeroScale), _baseYAtFullScale(baseYAtFullScale) {

	pos() = startPos;
	setTextures(_charAnimSet.textures());
	initScaling();
	setState(kWaiting); // also sets the animation
}

void Character::debugRender() {
	GameObject::debugRender();
	if (g_engine->console().debugFloor()) {
		const Point basePos(basePosX(), basePosY());
		const Point nearestToBase = g_engine->pathFinder().nearestWalkablePoint(basePos);
		if (basePos != nearestToBase)
			g_engine->renderer().debugLine(basePos, nearestToBase, 255, 0, 255);

		if (_state == kWalking) {
			const Point curPos = scaleSpritePosToBasePos(pos());
			const Point targetPos = scaleSpritePosToBasePos(_walkTarget);
			g_engine->renderer().debugLine(basePos, targetPos, 255, 0, 0);

			Point lastPos = targetPos;
			for (uint i = _walkPoints.size(); i > 0; i--) {
				Point waypoint = _walkPoints[i - 1];
				g_engine->renderer().debugRect(Rect(waypoint - Point(2, 2), 5, 5), 0, 0, 255);
				g_engine->renderer().debugLine(lastPos, waypoint, 0, 0, 255);
				lastPos = waypoint;
			}
		}
	}
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
			kTextDialog);
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

void Character::act(ActionModeId actionMode, uint32 duration) {
	_state = kActing;
	setAnimation(actionMode);
	_stateTimer.delay() = duration;
	_stateTimer.toggle(true);
}

bool Character::shutUp() {
	if (_state != kActing && _state != kTalking && _state != kThinking)
		return false;

	if (_talkText != nullptr)
		_talkText->toggle(false);
	_talkText = nullptr;
	g_system->getMixer()->stopHandle(_talkSound);
	_talkSound = {};
	setState(kWaiting);
	return true;
}

void Character::freeWalkTo(Point walkTo, Direction standbyDirection) {
	_standbyDirection = standbyDirection;
	_walkPoints.clear();
	_walkPoints.emplace_back(walkTo);
	nextWalkPoint();
}

void Character::pathWalkTo(Point walkTo, Direction standbyDirection) {
	_standbyDirection = standbyDirection;
	Point walkFrom = scaleSpritePosToBasePos(pos());
	if (walkFrom == walkTo || !g_engine->pathFinder().findPath(walkFrom, walkTo, _walkPoints))
		_walkPoints.clear();
	nextWalkPoint();
}

static Vector2d asVec(Point point) {
	return Vector2d(point.x, point.y);
}

static Point asPoint(Vector2d vec) {
	return Point((int16)(vec.getX()), (int16)(vec.getY()));
}

void Character::updateWalking() {
	if (_state != kWalking)
		return;

	Vector2d delta = asVec(_walkTarget) - _floatPos;
	Vector2d move = delta.getNormalized() * _cSpeed * g_engine->getElapsed();
	const auto absmin = [](float delta, float move) {
		return ABS(delta) < ABS(move) ? delta : move;
	};
	_floatPos += Vector2d(absmin(delta.getX(), move.getX()), absmin(delta.getY(), move.getY()));
	pos() = asPoint(_floatPos);

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
	Point signedDelta = _walkPoints.back() - scaleSpritePosToBasePos(pos());
	Point delta(ABS(signedDelta.x), ABS(signedDelta.y));
	_cSpeed = delta.x + delta.y == 0
		? 1.0f // we will arrive very soon at the destination where we already are
		: (_hSpeed * delta.x + _vSpeed * delta.y) / (delta.x + delta.y);
	_direction = delta.x > delta.y
		? (signedDelta.x < 0 ? Direction::Left : Direction::Right)
		: (signedDelta.y < 0 ? Direction::Up : Direction::Down);
	_floatPos = Vector2d(pos().x, pos().y);
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

Point Character::scaleSpritePosToBasePos(Point spritePos) const {
	float scale = 1.0f;
	if (_baseYAtZeroScale != _baseYAtFullScale)
		scale = MAX(0.001f, ABS((spritePos.y - _baseYAtZeroScale) * _scaleFactor));
	return spritePos + _baseOffset * scale;
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

Npc::Npc(
	Game &game,
	const DB::NPC &npc,
	RoomInteractionId interactionId,
	Point startPos,
	Point baseLineStart,
	Point baseLineEnd)
	: Character(game, startPos, npc._charAnimSet,
		npc._hspeed, npc._vspeed,
		npc._baseYAtZeroScale, npc._baseYAtFullScale,
		baseLineStart, baseLineEnd)
	, RoomInteractable(interactionId)
	, _name(npc._name) {
	_direction = Direction::Left;
	_talkFont = _thinkFont = npc._font;
}

const char *Npc::displayName() const {
	return _name;
}

void Npc::debugPrint() {
	g_engine->getDebugger()->debugPrintf("NPC \"%s\" (%s)\n", _name, stateToString());
}

Player::Player(Game &game, Point startPos, const DB::Room &room)
	: Character(game, startPos, room._charAnimSet,
		room._hspeed, room._vspeed,
		room._baseYAtZeroScale, room._baseYAtFullScale,
		Point(), Point()) {

	_talkFont = FontKind::EdnaFont;
	_thinkFont = FontKind::HarveyFont;
	if (room._gameMode == GameMode::Harvey)
		SWAP(_talkFont, _thinkFont);
}

void Player::debugPrint() {
	g_engine->getDebugger()->debugPrintf("Player (%s)\n", stateToString());
}

}

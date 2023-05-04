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

#include "topgun/topgun.h"

namespace TopGun {

ISpriteMessageHandler::ISpriteMessageHandler(Sprite *sprite, const SpriteMessage &message, SpriteMessageType expectedType) :
	_sprite(sprite),
	_script(sprite->getSpriteContext()->getEngine()->getScript()),
	_msg(message) {
	assert(_msg._type == expectedType);
	(void)expectedType;
}

void ISpriteMessageHandler::init() {
}

SpriteCellLoopHandler::SpriteCellLoopHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kCellLoop),
	_frameCount(0) {
}

void SpriteCellLoopHandler::init() {
	auto cellStart = _script->evalValue(_msg._cellLoop._cellStart);
	auto cellStop = _script->evalValue(_msg._cellLoop._cellStop);
	_frameCount = _sprite->setupCellAnimation(cellStart, cellStart, cellStop);
}

bool SpriteCellLoopHandler::update() {
	if (_sprite->_nextSpeedTrigger > g_system->getMillis())
		return false;
	if (!_frameCount)
		return true;

	if (_msg._cellLoop._duration._value != -1)
		_sprite->_motionDuration = _script->evalValue(_msg._cellLoop._duration);

	if (!_sprite->_priority || !_sprite->_nextSpeedTrigger)
		_sprite->_nextSpeedTrigger = g_system->getMillis();
	_sprite->_nextSpeedTrigger += _sprite->_motionDuration;
	_sprite->_setToNextCellOnRepaint = true;
	_sprite->_isVisible = true;
	return !--_frameCount && !_sprite->_motionDuration;
}

SpriteSetSubRectsHandler::SpriteSetSubRectsHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kSetSubRects),
	_hadBeenInit(false),
	_frameCount(0) {
}

void SpriteSetSubRectsHandler::init() {
	_frameCount = 0;
}

bool SpriteSetSubRectsHandler::update() {
	if (_sprite->_nextSpeedTrigger > g_system->getMillis())
		return false;
	if (_hadBeenInit)
		return true;

	if (_msg._subRects._duration._value != -1)
		_sprite->_motionDuration = _script->evalValue(_msg._subRects._duration);

	if (!_sprite->_priority || !_sprite->_nextSpeedTrigger)
		_sprite->_nextSpeedTrigger = g_system->getMillis();
	_sprite->_nextSpeedTrigger += _sprite->_motionDuration;

	_sprite->_subRects.resize(_msg._subRects._subRectCount);
	for (uint32 i = 0; i < _msg._subRects._subRectCount; i++) {
		auto cellIndex = _script->evalValue(_msg._subRects._subRectCells[i]);
		_sprite->_subRects[i]._bitmap = _sprite->_cells[cellIndex];
	}
	_sprite->setSubRectBounds();

	_sprite->_isVisible = true;
	_sprite->_setToNextCellOnRepaint = false;
	_hadBeenInit = true;
	return _sprite->_motionDuration == 0;
}

SpriteSetPriorityHandler::SpriteSetPriorityHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kSetPriority) {
}

bool SpriteSetPriorityHandler::update() {
	_sprite->_priority = _msg._priority;
	_sprite->_nextMotionTrigger = 0;
	_sprite->_nextSpeedTrigger = 0;
	if (_sprite->initNextMessage())
		return _sprite->updateMessage();
	return true;
}

SpriteOffsetAndFlipHandler::SpriteOffsetAndFlipHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kOffsetAndFlip) {
}

bool SpriteOffsetAndFlipHandler::update() {
	_sprite->_flipX = _script->evalValue(_msg._offsetAndFlip._flipX);
	_sprite->_flipY = _script->evalValue(_msg._offsetAndFlip._flipY);
	const Point offset(_script->evalValue(_msg._offsetAndFlip._offsetX), _script->evalValue(_msg._offsetAndFlip._offsetY));
	_sprite->translate(offset, true);

	if (!_sprite->_subRects.empty())
		_sprite->setSubRectBounds();
	return true;
}

SpriteHideHandler::SpriteHideHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kHide) {
}

bool SpriteHideHandler::update() {
	_sprite->setVisible(false);
	return true;
}

SpriteDelayHandler::SpriteDelayHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kDelay),
	_hasStarted(false) {
}

void SpriteDelayHandler::init() {
	_hasStarted = false;
}

bool SpriteDelayHandler::update() {
	if (_hasStarted)
		return _sprite->_nextSpeedTrigger <= g_system->getMillis();
	_hasStarted = true;

	if (!_sprite->_priority || !_sprite->_nextSpeedTrigger)
		_sprite->_nextSpeedTrigger = g_system->getMillis();
	_sprite->_nextSpeedTrigger += _script->evalValue(_msg._delay);
}

SpriteRunRootOpHandler::SpriteRunRootOpHandler(Sprite *sprite, const SpriteMessage &message) :
	ISpriteMessageHandler(sprite, message, SpriteMessageType::kRunRootOp) {
}

bool SpriteRunRootOpHandler::update() {
	_script->runQueueRootOp(_msg._rootOp, UINT32_MAX);
	return true;
}

}

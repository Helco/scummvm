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

#include "common/memstream.h"

#include "topgun/topgun.h"

using Common::SharedPtr;

namespace TopGun {

Sprite::Sprite(SpriteContext *spriteCtx, uint32 index) :
	IResource(kResourceType, index),
	_spriteCtx(spriteCtx) {
}

Sprite::~Sprite() {
	clearQueue();
}

bool Sprite::load(Common::Array<byte> &&data) {
	auto engine = _spriteCtx->getEngine();
	assert(engine->getResourceFile()->_architecture == Architecture::kBits32);
	constexpr uint32 kMinStoredResources = 8;

	auto stream = Common::MemorySeekableReadWriteStream(data.data(), data.size());
	stream.skip(8);
	const auto resourceCount = stream.readUint32LE();
	stream.skip(12);
	const auto colorCount = stream.readUint32LE();
	setLevel(stream.readSint32LE());
	stream.skip(1);
	_rectPickable = stream.readByte() != 0;
	stream.skip(1);
	const auto isTopMostSprite = stream.readByte() != 0;
	_pickableMode = (SpritePickableMode)stream.readByte();
	_isScrollable = stream.readByte() != 0;

	if (isTopMostSprite) {
		auto prevPos = stream.pos();
		stream.seek(MAX(kMinStoredResources, resourceCount) * sizeof(uint32), SEEK_CUR);
		_spriteCtx->setPaletteFromTopMostSprite(stream, colorCount);
		stream.seek(prevPos, SEEK_SET);
		_spriteCtx->getEngine()->setTopMostSprite(this);
	}

	_cells.reserve(resourceCount);
	for (size_t i = 0; i < resourceCount; i++) {
		auto resource = engine->loadResource(stream.readUint32LE(), ResourceType::kInvalid);
		const auto resourceType = resource->getResourceType();
		// TODO: Add special behaviour for Movie, Wave and Midi
		switch (resourceType) {
		case ResourceType::kBitmap:
		case ResourceType::kText:
		case ResourceType::kCell:
			addCell(resource.dynamicCast<ISurfaceResource>());
			break;
		}
		// TODO: Add special behaviour for text
	}

	return !stream.err();
}

void Sprite::render(Rect outBounds) {
	if (!_isVisible)
		return;
	if (_subRects.empty()) {
		renderSubRect(_cells[_curCellIndex], _bounds, outBounds);
	}
	else {
		for (auto &subRect : _subRects)
			renderSubRect(subRect._bitmap, subRect._bounds, outBounds);
	}
}

void Sprite::renderSubRect(Common::SharedPtr<ISurfaceResource> bitmap, Rect bounds, Rect outBounds) {
	auto clippedBounds = bounds;
	clippedBounds.clip(outBounds);
	if (_isScrollable)
		clippedBounds.clip(_spriteCtx->_clippedScrollBox);
	// TODO: Sprites can have their own clipping rect

	auto srcRect = Rect(clippedBounds.width(), clippedBounds.height());
	srcRect.translate(
		clippedBounds.left - bounds.left,
		clippedBounds.top - bounds.top);
	Point dstPos(
		clippedBounds.left - _spriteCtx->_screenBounds.left,
		clippedBounds.top - _spriteCtx->_screenBounds.top);
	_spriteCtx->_screen->transBlitFrom(*bitmap->getSurface(), srcRect, dstPos, 0, _flipX);
}

void Sprite::animate() {
	if (!_isEnabled || _curMessageIndex >= _queue.size())
		return;
	if (updateMessage())
		initNextMessage();
}

void Sprite::handleEnginePause(bool paused) {
	if (paused) {
		_wasPausedByGameplay = _paused;
		pause(true);
	}
	else if (!_wasPausedByGameplay)
		pause(false);
}

void Sprite::pause(bool paused) {
	if (_paused == paused)
		return;
	_paused = paused;
	if (paused)
		_timeAtPause = g_system->getMillis();
	else {
		const auto durationPaused = g_system->getMillis() - _timeAtPause;
		_nextSpeedTrigger += durationPaused;
		// TODO: original game does not increase nextMotionTrigger, but maybe it should?
	}
}

void Sprite::setLevel(int32 newLevel) {
	if (_level == newLevel)
		return;
	_level = newLevel;
	_spriteCtx->resortSprite(this);
}

void Sprite::addCell(SharedPtr<ISurfaceResource> resource) {
	_cells.push_back(resource);

	if (_cells.size() == 1) {
		_curCellIndex = 0;
		_cellIndexStop = 1;
		_animateCellsForward = true;
		setBoundsByCurrentCell();
	}
	else {
		if (_animateCellsForward && _cellIndexStop + 2 == _cells.size())
			_cellIndexStop++;
		if (!_animateCellsForward && _cellIndexStart + 2 == _cells.size())
			_cellIndexStart++;
	}
	_animateCell = _cellIndexStart != _cellIndexStop;
}

void Sprite::translate(Point target, bool relative) {
	setToNextCellIfNecessary();

	auto delta = target;
	if (relative)
		_pos += target;
	else {
		delta = _pos - target;
		_pos = target;
	}

	_bounds.translate(delta.x, delta.y);
	for (size_t i = 0; i < _subRects.size(); i++)
		_subRects[i]._bounds.translate(delta.x, delta.y);
}

void Sprite::setBoundsByCurrentCell() {
	_bounds = calcBoundsFor(_cells[_curCellIndex]);
}

void Sprite::setSubRectBounds() {
	for (size_t i = 0; i < _subRects.size(); i++) {
		auto subRectBounds = _subRects[i]._bounds = calcBoundsFor(_subRects[i]._bitmap);
		if (i)
			_bounds.extend(subRectBounds);
		else
			_bounds = subRectBounds;
	}
}

Rect Sprite::calcBoundsFor(Common::SharedPtr<ISurfaceResource> bitmap) {
	const auto surface = bitmap->getSurface();
	const auto offset = bitmap->getOffset();
	const auto halfWidth = (surface->w - 1) / 2;
	const auto halfHeight = (surface->h - 1) / 2;
	const auto xFactor = _flipX ? -1 : 1;
	const auto yFactor = _flipY ? -1 : 1;

	Rect bounds;
	bounds.left = _pos.x - halfWidth + xFactor * offset.x;
	bounds.right = bounds.left + surface->w;
	bounds.top = _pos.y - halfHeight + yFactor * offset.y;
	bounds.bottom = bounds.top + surface->h;

	if (_isScrollable)
		bounds.translate(_scrollPos.x, _scrollPos.y);
	return bounds;
}

void Sprite::transferTo(Common::SharedPtr<Sprite> dst) {
	dst->_curCellIndex = _curCellIndex;
	dst->_subRects = _subRects;
	dst->_isVisible = _isVisible;
	dst->_bounds = _bounds;
	dst->_pos = _pos;
	dst->_setToNextCellOnRepaint = false;

	_curCellIndex = 0;
	_subRects.clear();
	_isVisible = false;
	_setToNextCellOnRepaint = false;
}

uint32 Sprite::setupCellAnimation(uint32 nextCell, uint32 cellStart, uint32 cellStop) {
	if (cellStart >= _cells.size() || cellStop >= _cells.size())
		return 0;
	_cellIndexStart = cellStart;
	_cellIndexStop = cellStop;
	_animateCell = cellStart != cellStop;
	_animateCellsForward = cellStop >= cellStart;
	_setToNextCellOnRepaint = true;

	const auto minCell = MIN(cellStart, cellStop);
	const auto maxCell = MAX(cellStart, cellStop);
	_nextCellIndex = nextCell < minCell || nextCell > maxCell
		? cellStart
		: nextCell;
	return maxCell - minCell + 1;
}

void Sprite::setToNextCellIfNecessary() {
	if (!_setToNextCellOnRepaint)
		return;
	_subRects.clear();
	_setToNextCellOnRepaint = false;

	_curCellIndex = _nextCellIndex;
	if (_animateCellsForward) {
		if (++_nextCellIndex > _cellIndexStop)
			_nextCellIndex = _cellIndexStart;
	}
	else {
		if (_nextCellIndex == 0 || --_nextCellIndex < _cellIndexStop)
			_nextCellIndex = _cellIndexStart;
	}

	setBoundsByCurrentCell();
}

void Sprite::clearQueue() {
	for (auto handler : _queue)
		delete handler;
	_queue.clear();
	_curMessageIndex = UINT32_MAX;
	_motionDuration = 0;
	_nextMotionTrigger = 0;
	_speed = 0;
	_nextSpeedTrigger = 0;
	_priority = 0;
	_breakLoops = 0;
}

void Sprite::setQueue(const SpriteMessageQueue *queue) {
	clearQueue();
	_queue.reserve(queue->getMessageCount());
	for (size_t i = 0; i < queue->getMessageCount(); i++) {
		const auto &msg = queue->getMessage(i);
		_queue.push_back(ISpriteMessageHandler::create(this, msg));
	}

	_curMessageIndex = UINT32_MAX;
	initNextMessage();
}

void Sprite::sendMessage(const int32 *args, uint32 argCount) {
	SpriteMessage message(args, argCount);
	if (message._type == SpriteMessageType::kSetLoopMarker ||
		message._type == SpriteMessageType::kMessageLoop ||
		message._type == SpriteMessageType::kDelay)
		return;

	clearQueue();
	_queue.push_back(ISpriteMessageHandler::create(this, message));

	_curMessageIndex = UINT32_MAX;
	initNextMessage();
	updateMessage();
}

void Sprite::postMessage(const int32 *args, uint32 argCount) {
	SpriteMessage message(args, argCount);
	switch (message._type) {
	case (SpriteMessageType::kSetLoopMarker):
		_lastLoopMarker = _queue.size();
		break;
	case (SpriteMessageType::kMessageLoop):
		message._messageLoop._jumpIndex = _lastLoopMarker;
		break;
	}
	_queue.push_back(ISpriteMessageHandler::create(this, message));
}

bool Sprite::initNextMessage() {
	assert(_queue.size() > 0);
	_curMessageIndex++;

	// ugly special-casing, original, but maybe we find a way to replace this
	while (_curMessageIndex < _queue.size() &&
		_queue[_curMessageIndex]->getMessage()._type == SpriteMessageType::kMessageLoop) {
		if (_queue[_curMessageIndex]->update())
			_curMessageIndex = _queue[_curMessageIndex]->getMessage()._messageLoop._jumpIndex;
		else
			_curMessageIndex++;
	}

	if (_curMessageIndex >= _queue.size())
		clearQueue();
	else
		_queue[_curMessageIndex]->init();
	return _curMessageIndex < _queue.size();
}

bool Sprite::updateMessage() {
	return _queue[_curMessageIndex]->update();
}

void Sprite::setVisible(bool visible) {
	_isVisible = visible;
}

}

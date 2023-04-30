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
	_spriteCtx(spriteCtx),
	_isEnabled(true),
	_isVisible(false),
	_isScrollable(false),
	_animateCell(false),
	_animateCellsForward(false),
	_setToNextCellOnRepaint(false),
	_rectPickable(false),
	_cellIndexStart(0),
	_cellIndexStop(0),
	_curCellIndex(0),
	_level(0) {
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
	_spriteCtx->_screen->blitFrom(*bitmap->getSurface(), srcRect, dstPos);
}

void Sprite::animate() {

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

void Sprite::setBoundsByCurrentCell() {
	const auto cellSurface = _cells[_curCellIndex]->getSurface();
	auto center = _pos + _cells[_curCellIndex]->getOffset();
	if (_isScrollable)
		center += _scrollPos;
	_bounds = Rect::center(center.x, center.y, cellSurface->w, cellSurface->h);
}

}

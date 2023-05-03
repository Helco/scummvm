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

#ifndef TOPGUN_SPRITE_H
#define TOPGUN_SPRITE_H

#include "topgun/Resource.h"
#include "topgun/graphics/SpriteMessageHandler.h"

namespace TopGun {

enum class SpritePickableMode {
	kAlwaysPickable = 0,
	kPickableIfVisible,
	kNeverPickable
};

class SpriteContext;

struct SpriteSubRect {
	Common::SharedPtr<ISurfaceResource> _bitmap;
	Rect _bounds;
};

class Sprite : public IResource {
	friend class SpriteContext;
	friend class ISpriteMessageHandler;
	friend class SpriteCellLoopHandler;
	friend class SpriteSetSubRectsHandler;
	friend class SpriteCompToBackgroundHandler;
	friend class SpriteMoveCurveHandler;
	friend class SpriteMessageLoopHandler;
	friend class SpriteOffsetAndFlipHandler;
	friend class SpriteMoveLinearHandler;
	friend class SpriteDelayedMoveHandler;
	friend class SpriteDelayHandler;
	friend class SpriteSetPosHandler;
	friend class SpriteSetPriorityHandler;
	friend class SpriteSetRedrawHandler;
	friend class SpriteSetMotionDurationHandler;
	friend class SpriteSetCellAnimationHandler;
	friend class SpriteSetSpeedHandler;
	friend class SpriteShowCellHandler;
	friend class SpriteChangeSceneHandler;
	friend class SpriteRunRootOpHandler;
	friend class SpriteRunScriptHandler;
	friend class SpriteWaitForMovieHandler;

	Sprite(SpriteContext *spriteContext, uint32 index);
public:
	static constexpr ResourceType kResourceType = ResourceType::kSprite;
	virtual ~Sprite();

	virtual bool load(Common::Array<byte> &&data) override;

	void render(Rect outBounds);
	void animate();
	void translate(Point target, bool relative);
	void setLevel(int32 newLevel);
	void addCell(Common::SharedPtr<ISurfaceResource> resource);
	void clearQueue();
	void setQueue(const SpriteMessageQueue *queue);
	void setVisible(bool visible);

	inline SpriteContext *getSpriteContext() {
		return _spriteCtx;
	}
	inline Point getPos() const {
		return _pos;
	}
	inline Rect getBounds() const {
		return _bounds;
	}

private:
	void renderSubRect(Common::SharedPtr<ISurfaceResource> bitmap, Rect bounds, Rect outBounds);
	void setBoundsByCurrentCell();
	void setSubRectBounds();
	Rect calcBoundsFor(Common::SharedPtr<ISurfaceResource> bitmap);
	void transferTo(Common::SharedPtr<Sprite> dst);
	uint32 setupCellAnimation(uint32 nextCell, uint32 cellStart, uint32 cellStop); ///< returns frame count
	void setToNextCellIfNecessary();
	bool initNextMessage();
	bool updateMessage();

private:
	SpriteContext *_spriteCtx;
	Common::Array<Common::SharedPtr<ISurfaceResource> > _cells;
	Common::Array<SpriteSubRect> _subRects;
	Common::Array<ISpriteMessageHandler *> _queue;

	Point _pos;
	Point _scrollPos;
	Rect _bounds;

	bool _isEnabled;
	bool _isVisible;
	bool _isScrollable;
	bool _animateCell;
	bool _animateCellsForward;
	bool _setToNextCellOnRepaint;
	bool _rectPickable;
	bool _breakLoops;
	bool _priority;
	bool _flipX, _flipY;
	SpritePickableMode _pickableMode;
	uint32 _cellIndexStart, _cellIndexStop;
	uint32 _curCellIndex, _nextCellIndex;
	uint32 _curMessageIndex;
	uint32 _motionDuration, _nextMotionTrigger;
	uint32 _speed, _nextSpeedTrigger;
	int32 _level;
};

}

#endif // TOPGUN_SPRITE_H

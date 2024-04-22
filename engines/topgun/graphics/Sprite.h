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

	Sprite(SpriteContext *spriteContext, uint32 index, uint32 parentIndex);
public:
	static constexpr ResourceType kResourceType = ResourceType::kSprite;
	virtual ~Sprite();

	virtual bool load(Common::Array<byte> &&data) override;

	void render(Rect outBounds);
	void animate();
	void pause(bool pause);
	void handleEnginePause(bool pause);
	void translate(Point target, bool relative);
	void setLevel(int32 newLevel);
	void addCell(Common::SharedPtr<ISurfaceResource> resource);
	size_t getCellCount() const;
	void clearQueue();
	void setQueue(const SpriteMessageQueue *queue);
	bool setQueue(uint32 queueResIndex, bool hide = false);
	void sendMessage(const int32 *args, uint32 argCount);
	void postMessage(const int32 *args, uint32 argCount);
	void setBreakLoops(bool breakLoops);
	void setVisible(bool visible);

	void setClickable(bool toggle);
	void setClickScript(uint32 index);
	void setClickScriptArg(int32 arg);
	bool postClick(int32 arg0);

	bool isPickable() const;
	Common::SharedPtr<ISurfaceResource> pickCell(Point point) const;

	void printInfo();

	inline SpriteContext *getSpriteContext() {
		return _spriteCtx;
	}
	inline bool isVisible() const {
		return _isVisible;
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

	bool _isEnabled = true;
	bool _isVisible = false;
	bool _isScrollable = false;
	bool _isClickable = false;
	bool _isDraggable = false;
	bool _isRectPickable = false;
	bool _animateCell = false;
	bool _animateCellsForward = false;
	bool _setToNextCellOnRepaint = false;
	bool _breakLoops = false;
	bool _priority = false;
	bool _flipX = false, _flipY = false;
	bool _paused = false, _wasPausedByGameplay = false;
	SpritePickableMode _pickableMode = SpritePickableMode::kAlwaysPickable;
	uint32 _parentIndex = UINT32_MAX; // MAX meaning static sprite resource
	uint32 _cellIndexStart = 0, _cellIndexStop = 0;
	uint32 _curCellIndex = 0, _nextCellIndex = 0;
	uint32 _curMessageIndex = UINT32_MAX;
	uint32 _motionDuration = 0, _nextMotionTrigger = 0;
	uint32 _speed = 0, _nextSpeedTrigger = 0;
	uint32 _timeAtPause = 0;
	uint32 _lastLoopMarker = UINT32_MAX;
	uint32 _clickScriptIndex = 0;
	uint32 _dragScriptIndex = 0;
	int32 _clickScriptArg = 0;
	int32 _level = 0;
};

}

#endif // TOPGUN_SPRITE_H

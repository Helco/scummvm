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

#ifndef TOPGUN_SPRITEMESSAGEQUEUE_H
#define TOPGUN_SPRITEMESSAGEQUEUE_H

#include "topgun/Resource.h"

namespace TopGun {

enum class SpriteMessageType {
	kCellLoop = 1,
	kSetSubRects,
	kSetLoopMarker,
	kCompToBackground,
	kMoveCurve,
	kMessageLoop,
	kOffsetAndFlip,
	kHide,
	kMoveLinear,
	kDelayedMove,
	kDelay,
	kSetPos,
	kSetPriority,
	kSetRedraw,
	kSetMotionDuration,
	kSetCellAnimation,
	kSetSpeed,
	kShowCell,
	kFreeResources, // also unused
	kChangeScene,
	kRunRootOp,
	kRunScript,
	kWaitForMovie,
	kProc266
};

struct SpriteMessage {
	static constexpr int32 kMaxSubRects = 8;
	static constexpr int32 kMaxArgs = 6;

	SpriteMessage();
	SpriteMessage(const int32 *args, uint32 argCount);

	size_t _offset;
	SpriteMessageType _type;

	union {
		struct {
			ValueOrIndirect
				_cellStart,
				_cellStop,
				_duration;
		} _cellLoop;
		struct {
			ValueOrIndirect _duration,
				_subRectCells[kMaxSubRects];
			int32 _subRectCount;
		} _subRects;
		struct {
			bool _isRelative;
			ValueOrIndirect
				_duration,
				_point1X,
				_point1Y,
				_point2X,
				_point2Y;
		} _curve;
		struct {
			int32 _loopsRemaining,
				_loopCount,
				_jumpIndex;
		} _messageLoop;
		struct {
			ValueOrIndirect
				_flipX,
				_flipY,
				_offsetX,
				_offsetY;
		} _offsetAndFlip;
		struct {
			bool _isRelative,
				_durationIsSpeed;
			ValueOrIndirect
				_duration,
				_targetX,
				_targetY;
		} _linear;
		struct {
			bool _isRelative;
			ValueOrIndirect
				_targetX,
				_targetY;
		} _delayedMove, _pos;
		ValueOrIndirect _delay;
		int32 _priority,
			_redraw,
			_showCellIndex;
		ValueOrIndirect _motionDuration;
		struct {
			ValueOrIndirect
				_nextCell,
				_cellStart,
				_cellStop;
		} _cellAnimation;
		struct {
			ValueOrIndirect
				_speed,
				_duration;
		} _speed;
		struct {
			uint32 _resIndex,
				_argCount;
			int32 _args[kMaxArgs];
		} _script;
		struct {
			uint32 _resIndex;
			int32 _unk1;
			byte _unk2, _unk3;
		} _movie;
		struct {
			ValueOrIndirect _sprite, _flag;
		} _proc266;
	};

	Common::Array<byte> _rootOp;
};

class SpriteMessageQueue : public IResource {
public:
	static constexpr ResourceType kResourceType = ResourceType::kQueue;

	SpriteMessageQueue(uint32 index);
	virtual ~SpriteMessageQueue() = default;

	virtual bool load(Common::Array<byte> &&data) override;

	inline size_t getMessageCount() const {
		return _messages.size();
	}

	inline const SpriteMessage &getMessage(size_t i) const {
		return _messages[i];
	}

private:
	Common::Array<SpriteMessage> _messages;
};

}

#endif // TOPGUN_SPRITEMESSAGEQUEUE_H

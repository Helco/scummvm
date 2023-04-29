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

#include "topgun/graphics/SpriteMessageQueue.h"
#include "topgun/topgun.h"

namespace TopGun {

SpriteMessageQueue::SpriteMessageQueue(uint32 index) : IResource(kResourceType, index) {
}

/* The original games read the messages as raw structures which also include
 * fields only used during runtime. Thus this code contains a lot of skips.
 */

bool SpriteMessageQueue::load(Common::Array<byte> &&data) {
	auto architecture = g_engine->getResourceFile()->_architecture;
	assert(architecture == Architecture::kBits32);
	auto stream = Common::MemorySeekableReadWriteStream(data.data(), data.size());

	while (stream.pos() < stream.size() && !stream.err()) {
		SpriteMessage msg;
		msg._offset = stream.pos();
		msg._type = (SpriteMessageType)stream.readUint16LE();
		switch (msg._type) {
		case SpriteMessageType::kUnused3:
		case SpriteMessageType::kCompToBackground:
		case SpriteMessageType::kHide:
		case SpriteMessageType::kChangeScene:
			// no arguments to read
			break;
		case SpriteMessageType::kCellLoop:
			msg._cellLoop._cellStart._value = stream.readSint32LE();
			msg._cellLoop._cellStop._value = stream.readSint32LE();
			stream.skip(4);
			msg._cellLoop._duration._value = stream.readSint32LE();
			msg._cellLoop._cellStart._isIndirect = stream.readByte() != 0;
			msg._cellLoop._cellStop._isIndirect = stream.readByte() != 0;
			msg._cellLoop._duration._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			break;
		case SpriteMessageType::kSetSubRects: {
			msg._subRects._duration._value = stream.readSint32LE();
			const auto subRectCount = msg._subRects._subRectCount = stream.readSint32LE();
			assert(subRectCount > 0 && subRectCount <= SpriteMessage::kMaxSubRects);
			stream.skip(1);
			msg._subRects._duration._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			const auto subRectIsIndirect = stream.readByte();
			for (int32 i = 0; i < subRectCount; i++) {
				msg._subRects._subRectCells[i]._value = stream.readSint32LE();
				msg._subRects._subRectCells[i]._isIndirect = subRectIsIndirect & (1 << i);
			}
		}break;
		case SpriteMessageType::kMoveCurve:
			msg._curve._duration._value = stream.readSint32LE();
			msg._curve._isRelative = stream.readByte() != 0;
			msg._curve._point1X._isIndirect = stream.readByte() != 0;
			msg._curve._point1Y._isIndirect = stream.readByte() != 0;
			msg._curve._point2X._isIndirect = stream.readByte() != 0;
			msg._curve._point2Y._isIndirect = stream.readByte() != 0;
			msg._curve._duration._isIndirect = stream.readByte() != 0;
			msg._curve._point1X._value = stream.readSint32LE();
			msg._curve._point1Y._value = stream.readSint32LE();
			msg._curve._point2X._value = stream.readSint32LE();
			msg._curve._point2Y._value = stream.readSint32LE();
			stream.skip(8 * sizeof(int32));
			break;
		case SpriteMessageType::kMessageLoop:
			msg._messageLoop._loopsRemaining = stream.readSint32LE();
			msg._messageLoop._loopCount = stream.readSint32LE();
			msg._messageLoop._jumpIndex = stream.readSint32LE();
			// currently still offset, we will fix this after reading all messages
			break;
		case SpriteMessageType::kOffsetAndFlip:
			msg._offsetAndFlip._flipX._value = stream.readSint32LE();
			msg._offsetAndFlip._flipY._value = stream.readSint32LE();
			msg._offsetAndFlip._offsetX._isIndirect = stream.readByte() != 0;
			msg._offsetAndFlip._offsetY._isIndirect = stream.readByte() != 0;
			msg._offsetAndFlip._flipX._isIndirect = stream.readByte() != 0;
			msg._offsetAndFlip._flipY._isIndirect = stream.readByte() != 0;
			msg._offsetAndFlip._offsetX._value = stream.readSint32LE();
			msg._offsetAndFlip._offsetY._value = stream.readSint32LE();
			break;
		case SpriteMessageType::kMoveLinear:
			msg._linear._duration._value = stream.readSint32LE();
			msg._linear._isRelative = stream.readByte() != 0;
			msg._linear._durationIsSpeed = stream.readByte() != 0;
			msg._linear._targetX._isIndirect = stream.readByte() != 0;
			msg._linear._targetY._isIndirect = stream.readByte() != 0;
			msg._linear._duration._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			msg._linear._targetX._value = stream.readSint32LE();
			msg._linear._targetY._value = stream.readSint32LE();
			stream.skip(6 * sizeof(int32));
			break;
		case SpriteMessageType::kDelayedMove:
			msg._delayedMove._isRelative = stream.readByte() != 0;
			msg._delayedMove._targetX._isIndirect = stream.readByte() != 0;
			msg._delayedMove._targetY._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			msg._delayedMove._targetX._value = stream.readSint32LE();
			msg._delayedMove._targetY._value = stream.readSint32LE();
			break;
		case SpriteMessageType::kDelay:
			msg._delay._value = stream.readSint32LE();
			stream.skip(1);
			msg._delay._isIndirect = stream.readByte() != 0;
			break;
		case SpriteMessageType::kSetPos:
			msg._pos._targetX._value = stream.readSint32LE();
			msg._pos._targetY._value = stream.readSint32LE();
			msg._pos._isRelative = stream.readByte() != 0;
			msg._pos._targetX._isIndirect = stream.readByte() != 0;
			msg._pos._targetY._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			break;
		case SpriteMessageType::kSetPriority:
			msg._priority = stream.readSint32LE();
			break;
		case SpriteMessageType::kSetRedraw:
			msg._redraw = stream.readSint32LE();
			break;
		case SpriteMessageType::kShowCell:
			msg._showCellIndex = stream.readSint32LE();
			break;
		case SpriteMessageType::kSetMotionDuration:
			msg._motionDuration._value = stream.readSint32LE();
			msg._motionDuration._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			break;
		case SpriteMessageType::kSetCellAnimation:
			msg._cellAnimation._nextCell._value = stream.readSint32LE();
			msg._cellAnimation._cellStart._value = stream.readSint32LE();
			msg._cellAnimation._cellStop._value = stream.readSint32LE();
			msg._cellAnimation._nextCell._isIndirect = stream.readByte() != 0;
			msg._cellAnimation._cellStart._isIndirect = stream.readByte() != 0;
			msg._cellAnimation._cellStop._isIndirect = stream.readByte() != 0;
			stream.skip(1);
			break;
		case SpriteMessageType::kSetSpeed:
			msg._speed._speed._value = stream.readSint32LE();
			msg._speed._duration._value = stream.readSint32LE();
			msg._speed._speed._isIndirect = stream.readByte() != 0;
			msg._speed._duration._isIndirect = stream.readByte() != 0;
			break;
		case SpriteMessageType::kRunRootOp:
			msg._rootOp.resize(stream.readUint32LE());
			stream.read(msg._rootOp.data(), msg._rootOp.size());
			break;
		case SpriteMessageType::kRunScript: {
			msg._script._resIndex = stream.readUint32LE();
			const auto argCount = msg._script._argCount = stream.readUint32LE();
			assert(argCount < SpriteMessage::kMaxArgs);
			for (uint32 i = 0; i < argCount; i++)
				msg._script._args[i] = stream.readSint32LE();
			stream.skip((SpriteMessage::kMaxArgs - argCount) * sizeof(int32));
		}break;
		case SpriteMessageType::kWaitForMovie:
			msg._movie._resIndex = stream.readUint32LE();
			msg._movie._unk1 = stream.readSint32LE();
			msg._movie._unk2 = stream.readByte();
			msg._movie._unk3 = stream.readByte();
		case SpriteMessageType::kProc266:
			msg._proc266._sprite._value = stream.readSint32LE();
			msg._proc266._flag._value = stream.readSint32LE();
			msg._proc266._sprite._isIndirect = stream.readByte() != 0;
			msg._proc266._flag._isIndirect = stream.readByte() != 0;
			break;
		default: error("Unsupported sprite message %d", msg._type);
		}
		_messages.push_back(msg);
	}

	// The original parses the queue bytes during interpretation and as such
	// just uses a byte offset for the MessageLoop message.
	// We do not and as such have to convert the offset to an index
	for (size_t i = 0; i < _messages.size(); i++) {
		if (_messages[i]._type != SpriteMessageType::kMessageLoop)
			continue;
		const auto offset = _messages[i]._messageLoop._jumpIndex;
		size_t j;
		for (j = 0; j < _messages.size(); j++) {
			if (_messages[j]._offset == offset)
				break;
		}
		if (j >= _messages.size())
			error("Invalid loop jump offset");
		_messages[i]._messageLoop._jumpIndex = j;
	}

	return !stream.err();
}

}

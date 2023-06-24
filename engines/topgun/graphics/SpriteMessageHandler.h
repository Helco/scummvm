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

#ifndef TOPGUN_SPRITEMESSAGEHANDLER_H
#define TOPGUN_SPRITEMESSAGEHANDLER_H

#include "SpriteMessageQueue.h"

namespace TopGun {

class Sprite;
class Script;

class ISpriteMessageHandler {
public:
	ISpriteMessageHandler(Sprite *sprite, const SpriteMessage &message, SpriteMessageType expectedType);
	virtual ~ISpriteMessageHandler() = default;

	static ISpriteMessageHandler *create(Sprite *sprite, const SpriteMessage &message);

	virtual void init();
	virtual bool update() = 0; ///< returns true if message is done

	inline const SpriteMessage &getMessage() const {
		return _msg;
	}
protected:
	Sprite *_sprite;
	Script *_script;
	SpriteMessage _msg;
};

class SpriteCellLoopHandler : public ISpriteMessageHandler {
public:
	SpriteCellLoopHandler(Sprite *sprite, const SpriteMessage &message);

	virtual void init() override;
	virtual bool update() override;

private:
	uint32 _frameCount = 0;
};

class SpriteSetSubRectsHandler : public ISpriteMessageHandler {
public:
	SpriteSetSubRectsHandler(Sprite *sprite, const SpriteMessage &message);

	virtual void init() override;
	virtual bool update() override;

private:
	bool _hadBeenInit = false;
	uint32 _frameCount = 0;
};

class SpriteOffsetAndFlipHandler : public ISpriteMessageHandler {
public:
	SpriteOffsetAndFlipHandler(Sprite *sprite, const SpriteMessage &message);

	virtual bool update() override;
};

class SpriteHideHandler : public ISpriteMessageHandler {
public:
	SpriteHideHandler(Sprite *sprite, const SpriteMessage &message);

	virtual bool update() override;
};

class SpriteDelayHandler : public ISpriteMessageHandler {
public:
	SpriteDelayHandler(Sprite *sprite, const SpriteMessage &message);

	virtual void init() override;
	virtual bool update() override;

private:
	bool _hasStarted = false;
};

class SpriteSetPriorityHandler : public ISpriteMessageHandler {
public:
	SpriteSetPriorityHandler(Sprite *sprite, const SpriteMessage &message);

	virtual bool update() override;
};

class SpriteRunRootOpHandler : public ISpriteMessageHandler {
public:
	SpriteRunRootOpHandler(Sprite *sprite, const SpriteMessage &message);

	virtual bool update() override;
};

class SpriteRunScriptHandler : public ISpriteMessageHandler {
public:
	SpriteRunScriptHandler(Sprite *sprite, const SpriteMessage &message);

	virtual bool update() override;
};

}

#endif // TOPGUN_SPRITEMESSAGEHANDLER_H

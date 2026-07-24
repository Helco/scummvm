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

#include "edna/assetcache.h"
#include "edna/edna.h"
#include "edna/db.h"
#include "edna/game/game.h"
#include "edna/sprite/animation.h"
#include "edna/sprite/npc.h"
#include "edna/sprite/player.h"

using namespace Common;

namespace Edna {

GameBase::GameBase(ScopedPtr<GameBase> &myPtr, GameMode mode) : _gameMode(mode) {
	// we set ourselves into the pointer, so the instance is registered very early
	// that way if there is some error, the console can already access it and inspect
	myPtr.reset(this);
}

GameBase::~GameBase() { }

void GameBase::update() {
	for (auto &group : _groups)
		group->update();
}

void GameBase::render() {
	for (auto &group : _groups)
		group->render();
}

void GameBase::add(Group *group, DisposeAfterUse::Flag dispose) {
	_groups.emplace_back(group, dispose);
}

Game::Game(ScopedPtr<GameBase> &myPtr, GameMode mode, RoomId roomId)
	: GameBase(myPtr, mode)
	, _roomId(roomId)
	, _script(*this)
	, _background("background")
	, _bgObjects("bgObjects")
	, _objects("objects")
	, _texts("texts")
	, _gui("gui") {
	assert(mode != GameMode::Intro);
	DB::Room room = g_engine->db().room(roomId);
	assert(room._gameMode == mode);

	initBackground(room._background);
	initTimer(room._timer);
	initGroups();
	// TODO: Init walkableareamap
	// TODO: Init font and cursor
	// TODO: Init comment
	initPlayer();
	initObjects();
	// TODO: Init CommandPrompt
	// TODO: Init ScriptInterpreter
	// TODO: Init music
}

void Game::initBackground(const char *background) {
	auto backgroundSprite = new Sprite();
	backgroundSprite->setTexture(background);
	_background.add(backgroundSprite, DisposeAfterUse::YES);
}

void Game::initTimer(TimerId timerId) {
	if (timerId == 0)
		return;
	const auto dbTimer = g_engine->db().timer(timerId);
	_timerScript = dbTimer._script;
	_timer.delay() = dbTimer._duration;
	_timer.toggle(dbTimer._active);
}

void Game::initGroups() {
	initGroups(nullptr, nullptr);
}

void Game::initGroups(Group *specialObjects, Group *specialGui) {
	add(&_background);
	add(&_bgObjects);
	add(&_objects);
	if (specialObjects != nullptr)
		add(specialObjects);
	add(&_texts);
	add(&_gui);
	// TODO: comment
	if (specialGui != nullptr)
		add(specialGui);
	// TODO: start menu
}

void Game::initPlayer() {
	_player = new Player(Point(), g_engine->db().room(_roomId));
	_player->immutable() = true;
	_objects.add(_player, DisposeAfterUse::YES);
}

void Game::initObjects() {
	auto dbObjects = g_engine->db().roomObjectsByRoom(_roomId, false);
	for (uint32 dbObjectId : dbObjects) {
		const auto dbObject = g_engine->db().roomObject(dbObjectId);
		// not sure what the logic behind this is, display is still ignored for NPC/Exit/Interaction
		Group &targetGroup = dbObject._toDisplay != 0 ? _objects : _bgObjects;
		Sprite *sprite = nullptr;

		if (dbObject._toNPC != 0) {
			const auto dbNpc = g_engine->db().npc(dbObject._toNPC);
			const auto dbDisplay = g_engine->db().roomObjectDisplay(dbObject._toDisplay, false);
			auto *npc = new Npc(
				dbNpc,
				dbObject._toInteraction,
				{ (int16)dbObject._posX, (int16)dbObject._posY },
				{ (int16)dbDisplay._startX, (int16)dbDisplay._startY },
				{ (int16)dbDisplay._endX, (int16)dbDisplay._endY });
			sprite = npc;
		}
		else if (dbObject._toInteraction != 0) {
			const auto dbInteraction = g_engine->db().roomInteraction(dbObject._toInteraction);
			if (dbInteraction._toExit) {
				warning("Exit is not supported (room=%u, object=%u, interaction=%u, exit=%u)",
					_roomId, dbObjectId, dbObject._toInteraction, dbInteraction._toExit);
				continue;
			}
		}
		else if (dbObject._toDisplay != 0) {
			const auto dbDisplay = g_engine->db().roomObjectDisplay(dbObject._toDisplay);
			auto *object = new VisualObject(
				{ (int16)dbDisplay._startX, (int16)dbDisplay._startY },
				{ (int16)dbDisplay._endX, (int16)dbDisplay._endY });
			sprite = object;
			if (dbDisplay._animation > 0) {
				Animation animation(dbDisplay._animation);
				object->setAnimation(animation);
			} else
				object->setTexture(dbObject._image);
			object->setBasePos({ (int16)dbObject._posX, (int16)dbObject._posY });
		}
		// else this is a pure logic object used by scripts

		if (sprite != nullptr) {
			sprite->id() = dbObjectId;
			sprite->immutable() = true;
			sprite->toggle(dbObject._active);
			targetGroup.add(sprite, DisposeAfterUse::YES);
		}
	}
}

void Game::update() {
	updateFade();
	if (!updateScript())
		return;
	GameBase::update(); // updates sprite groups
}

void Game::render() {
	GameBase::render();
	if (_fade._current > 0.0f) {
		g_engine->renderer().rect({ 0, 0, kScreenWidth, kScreenHeight }, // this might be the wrong blending
			_fade._color, _fade._color, _fade._color, (uint8)CLIP(_fade._current * 255.0f, 0.0f, 255.0f));
	}
}

void Game::updateFade() {
	if (!_fade._active)
		return;
	_fade._current += _fade._speed * g_engine->getElapsedF();
	if (_fade._current < 0.0f || _fade._current > 1.0f) {
		_fade._current = CLIP(_fade._current, 0.0f, 1.0f);
		_fade._active = false;
	}
}

bool Game::updateScript() {
	// TODO: Missing continue after savegame load
	// TODO: Missing continue choice list after savegame load
	// TODO: Missing continue script across rooms

	if (_script.isScriptRunning()) {
		if (!_script.isPerforming())
			_script.continueScript();
	} else {
		// TODO: Enable cursor
		if (_pendingTimerInvoke) {
			debugC(0, kDebugScript, "Invoke timer script %u", _timerScript);
			_pendingTimerInvoke = false;
			_timer.toggle(false);
			_script.runNewScript(_timerScript);
		}
	}

	if (_timer.update())
		_pendingTimerInvoke = true;
	return true;
}

Sprite *Game::objectById(RoomObjectId id) const {
	auto sprite = _objects.byId(id);
	if (sprite == nullptr)
		sprite = _bgObjects.byId(id);
	return sprite;
}

void Game::fade(byte color, float target, uint32 duration) {
	_fade._active = true;
	_fade._color = color;
	_fade._speed = (target - _fade._current) * 1000.0f / duration;
}

}

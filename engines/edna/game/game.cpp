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
#include "edna/console.h"
#include "edna/edna.h"
#include "edna/db.h"
#include "edna/game/game.h"
#include "edna/graphics.h"
#include "edna/input.h"
#include "edna/pathfinder.h"
#include "edna/sprite/animation.h"
#include "edna/sprite/character.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

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

void GameBase::debugRender() {
	if (!g_engine->console().anyDebugDraw())
		return;
	for (auto &group : _groups)
		group->debugRender();

	if (g_engine->console().debugFloor()) {
		Point mousePos = g_engine->input().mousePos();
		Point nearest = g_engine->pathFinder().nearestWalkablePoint(mousePos);
		if (mousePos != nearest)
			g_engine->renderer().debugLine(mousePos, nearest, 255, 0, 255);
	}
}

void GameBase::add(Group *group, DisposeAfterUse::Flag dispose) {
	_groups.emplace_back(group, dispose);
}

Game::Game(ScopedPtr<GameBase> &myPtr, GameMode mode, const GameTransition &transition)
	: GameBase(myPtr, mode)
	, _roomId(transition._room)
	, _script(*this, transition)
	, _background("background")
	, _bgObjects("bgObjects")
	, _objects("objects")
	, _texts("texts")
	, _gui("gui") {
	assert(mode != GameMode::Intro);
}

void Game::init(const GameTransition &transition) {
	DB::Room room = g_engine->db().room(_roomId);
	assert(room._gameMode == gameMode());
	initBackground(room._background);
	initTimer(room._timer);
	initGroups();
	initPathFinder(room._walkAreaId);
	// TODO: Init comment
	initPlayer(transition);
	initObjects();
	g_engine->playMusic(room._music);
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

void Game::initPathFinder(WalkableAreaId areaId) {
	const auto area = g_engine->db().walkableArea(areaId);
	g_engine->pathFinder().loadArea(area._file);
}

void Game::initPlayer(const GameTransition &transition) {
	_player = new Player(*this, transition._walkIn, g_engine->db().room(_roomId));
	_player->immutable() = true;
	if (transition._walkInDir != Direction::None)
		_player->direction() = transition._walkInDir;
	_objects.add(_player, DisposeAfterUse::YES);
}

void Game::initObjects() {
	auto dbObjects = g_engine->db().roomObjectsByRoom(_roomId, false);
	for (uint32 dbObjectId : dbObjects) {
		const auto dbObject = g_engine->db().roomObject(dbObjectId);
		const auto dbDisplay = g_engine->db().roomObjectDisplay(dbObject._toDisplay, false);
		// not sure what the logic behind this is, display is still ignored for NPC/Exit/Interaction
		Group &targetGroup = dbObject._toDisplay != 0 ? _objects : _bgObjects;
		GameObject *sprite;

		if (dbObject._toNPC != 0) {
			const auto dbNpc = g_engine->db().npc(dbObject._toNPC);

			auto *npc = new Npc(
				*this,
				dbNpc,
				dbObject._toInteraction,
				dbObject._pos,
				dbDisplay._baseLineStart, dbDisplay._baseLineEnd);
			sprite = npc;
		} else if (dbObject._toInteraction == 0 && dbObject._toDisplay == 0)
			continue; // this is a pure logic object used by scripts
		else {
			if (dbObject._toInteraction != 0) {
				const auto dbInteraction = g_engine->db().roomInteraction(dbObject._toInteraction);
				if (dbInteraction._toExit != 0) {
					sprite = new RoomExit(
						dbInteraction._id, dbInteraction._toExit,
						dbDisplay._baseLineStart, dbDisplay._baseLineEnd);
				} else {
					sprite = new InteractableRoomObject(
						dbInteraction._id,
						dbDisplay._baseLineStart, dbDisplay._baseLineEnd);
				}
			} else
				sprite = new RoomObject(dbDisplay._baseLineStart, dbDisplay._baseLineEnd);

			if (dbDisplay._animation != 0)
				sprite->setAnimation(Animation(dbDisplay._animation));
			else
				sprite->setTexture(dbObject._image);
			sprite->setBasePos(dbObject._pos);
		}

		sprite->id() = dbObjectId;
		sprite->immutable() = true;
		sprite->toggle(dbObject._active);
		targetGroup.add(sprite, DisposeAfterUse::YES);
	}
}

void Game::update() {
	updateFade();
	if (!updateScript())
		return;
	CursorMan.showMouse(!script().isScriptRunning() && !script().isPerforming());
	g_engine->assets().useStandardCursor();

	GameBase::update(); // updates sprite groups
	updateInput();
}

void Game::render() {
	GameBase::render();
	if (_fade._current > 0.0f) {
		g_engine->renderer().rect({ 0, 0, kScreenWidth, kScreenHeight }, // this might be the wrong blending
			_fade._color, _fade._color, _fade._color, (uint8)CLIP(_fade._current * 255.0f, 0.0f, 255.0f));
	}
}

void Game::debugRender() {
	GameBase::debugRender();
	if (g_engine->console().debugFloor()) {
		createDebugFloorTexture();
		g_engine->renderer().sprite(_debugFloorTexture.get(), Point());
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
			_script.resume();
	} else {
		// TODO: Enable cursor
		if (_pendingTimerInvoke) {
			debugC(0, kDebugScript, "Invoke timer script %u", _timerScript);
			_pendingTimerInvoke = false;
			_timer.toggle(false);
			_script.runNew(_timerScript);
		}
	}

	if (_timer.update())
		_pendingTimerInvoke = true;
	return true;
}

void Game::updateInput() {
	if (g_engine->input().wasMouseRightReleased() && shutUp())
		g_engine->input().nextFrame(); // otherwise the click triggers the next default action
}

bool Game::shutUp() {
	bool result = false;
	for (const auto &sprite : _objects.sprites()) {
		Character *character = dynamic_cast<Character *>(sprite.get());
		if (character != nullptr)
			result |= character->shutUp();
	}
	for (const auto &sprite : _bgObjects.sprites()) {
		Character *character = dynamic_cast<Character *>(sprite.get());
		if (character != nullptr)
			result |= character->shutUp();
	}
	result |= script().shutUp();
	return result;
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

void Game::triggerExit(RoomExitId exitId, ScriptId scriptId, uint32 scriptLine) {
	const auto dbExit = g_engine->db().roomExit(exitId);
	auto &next = g_engine->next();
	next._room = dbExit._target;
	next._walkIn = dbExit._walkIn;
	next._walkInDir = dbExit._lookDirection;
	next._script = scriptId;
	next._scriptLine = scriptLine;
}

void Game::triggerChoiceList(ChoiceSetId setId) {
	warning("Triggered choice list %u in unsupported game mode %s", setId, gameModeToString(gameMode()));
}

void Game::triggerInventoryUpdate() {
}

void Game::createDebugFloorTexture() {
	if (_debugFloorTexture != nullptr)
		return;
	// Manual copy because we have to transpose as well
	const auto src = g_engine->pathFinder().map();
	Graphics::ManagedSurface dst(kScreenWidth, kScreenHeight, g_system->getScreenFormat());
	const uint32 color = dst.format.ARGBToColor(100, 0, 255, 0);
	for (uint y = 0; y < kScreenHeight; y++) {
		uint32 *dstLine = (uint32 *)(dst.getBasePtr(0, y));
		for (uint x = 0; x < kScreenWidth; x++)
			*(dstLine++) = src[x * kScreenHeight + y] ? color : 0;
	}
	_debugFloorTexture = g_engine->renderer().loadTexture(dst);
}

}

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

#include "edna/edna.h"
#include "edna/db.h"
#include "edna/game/game.h"
#include "edna/sprite/animation.h"
#include "edna/sprite/object.h"

namespace Edna {

GameBase::GameBase(GameMode mode) : _gameMode(mode) { }
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

Game::Game(GameMode mode, RoomId roomId)
	: GameBase(mode)
	, _roomId(roomId)
	, _background("background")
	, _bgObjects("bgObjects")
	, _objects("objects")
	, _texts("texts")
	, _gui("gui") {
	assert(mode != GameMode::Intro);
	DB::Room room = g_engine->db().room(roomId);
	assert(room._gameMode == mode);

	initBackground(room._background);
	// TODO: Init timer
	initGroups();
	// TODO: Init walkableareamap
	// TODO: Init font and cursor
	// TODO: Init comment
	// TODO: Init characterAnimationSet
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

void Game::initObjects() {
	auto dbObjects = g_engine->db().roomObjectsByRoom(_roomId, false);
	for (uint32 dbObjectId : dbObjects) {
		const auto dbObject = g_engine->db().roomObject(dbObjectId);
		// not sure what the logic behind this is, display is still ignored for NPC/Exit/Interaction
		Group &targetGroup = dbObject._toDisplay != 0 ? _objects : _bgObjects;

		if (dbObject._toNPC != 0) {
			warning("NPC is not supported (room=%u, object=%u, npc=%u)", _roomId, dbObjectId, dbObject._toNPC);
			continue;
		}

		if (dbObject._toInteraction != 0) {
			const auto dbInteraction = g_engine->db().roomInteraction(dbObject._toInteraction);
			if (dbInteraction._toExit) {
				warning("Exit is not supported (room=%u, object=%u, interaction=%u, exit=%u)",
					_roomId, dbObjectId, dbObject._toInteraction, dbInteraction._toExit);
				continue;
			}
		}

		if (dbObject._toDisplay != 0) {
			const auto dbDisplay = g_engine->db().roomObjectDisplay(dbObject._toDisplay);
			auto *object = new VisualObject(
				{ (int16)dbDisplay._startX, (int16)dbDisplay._startY },
				{ (int16)dbDisplay._endX, (int16)dbDisplay._endY });
			object->pos() = { (int16)dbObject._posX, (int16)dbObject._posY };
			object->active() = dbObject._active;
			object->immutable() = true;
			object->id() = dbObjectId;
			if (dbDisplay._animation > 0) {
				Animation animation(dbDisplay._animation);
				object->setAnimation(animation);
			} else
				object->setTexture(dbObject._image);
			targetGroup.add(object, DisposeAfterUse::YES);
			continue;
		}

		// if we get here this is a pure logic object used by scripts
	}
}

}

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
#include "edna/db.h"
#include "edna/edna.h"
#include "edna/input.h"
#include "edna/game/harvey.h"
#include "edna/sprite/character.h"

using namespace Common;

namespace Edna {

Harvey::Harvey(ScopedPtr<GameBase> &myPtr, const GameTransition &transition)
	: Game(myPtr, GameMode::Harvey, transition)
	, _commandPrompt(*this)
	, _choiceList(*this)
	, _topicRow(transition._room)
	, _buttonToEdna(1, Point(698, 534), "gui/harvey/b_zuedna")
	, _pastIds(PastRoomIds::fromHarveyRoom(transition._room)) {

	if (g_engine->db().roomObject(_pastIds._ednaId, false)._id == 0) {
		_inactiveButtonToEdna.pos() = Point(698, 534);
		_inactiveButtonToEdna.setTexture("gui/harvey/b_zuedna_i.png");
		gui().add(&_inactiveButtonToEdna, DisposeAfterUse::NO);
	} else
		gui().add(&_buttonToEdna, DisposeAfterUse::NO);
	gui().add(&_commandPrompt, DisposeAfterUse::NO);

	init(transition);
}

void Harvey::initGroups() {
	Game::initGroups(&_topicRow, &_choiceList);
}

void Harvey::triggerChoiceList(ChoiceSetId setId) {
	_choiceList.openSet(setId);
}

void Harvey::update() {
	Game::update();

	if (script().isScriptRunning() || script().isPerforming() || _choiceList.active())
		return;

	Sprite *selection = findSelection();

	if (_command._isComplete) {
		if (player().state() == Character::kWaiting) {
			selection = nullptr;
			invokeCompletedCommand();
		}
	} else
		updateHover(selection);

	const auto input = g_engine->input();
	if (input.wasMouseLeftPressed())
		onMouseLeftPressed(selection);
	if (input.wasMouseLeftReleased())
		onMouseLeftReleased(selection);
	if (input.wasMouseRightPressed() && selection == nullptr)
		_command = {}; // TODO: What about while dragging?
	if (input.wasMouseRightReleased())
		onMouseRightReleased(selection);
	if (input.isMouseLeftPressed() && _dragStatus == DragStatus::StartDrag)
		onStartDrag(selection);
	if (_dragStatus == DragStatus::StartDrop)
		onStartDrop(selection);

	_commandPrompt.setText(_command, selection);
}

bool Harvey::isTopic(Sprite *sprite) const {
	return sprite != nullptr && &sprite->group() == &_topicRow && sprite->id() > 0;
}

bool Harvey::isTopicRow(Sprite *sprite) const {
	return sprite != nullptr && &sprite->group() == &_topicRow && sprite->id() == 0;
}

Sprite *Harvey::findSelection() {
	const Point mousePos = g_engine->input().mousePos();
	Sprite *sprite = gui().checkClick(mousePos);
	if (sprite == nullptr)
		sprite = _topicRow.checkClick(mousePos);
	if (sprite == nullptr)
		sprite = objects().checkInteractableClick(mousePos);
	if (sprite == nullptr)
		sprite = bgObjects().checkClick(mousePos);

	// While dragging selection only reacts to the topic row and Edna
	if (sprite != nullptr && _dragStatus == DragStatus::Dragging &&
		sprite->id() != _pastIds._ednaId && !isTopicRow(sprite))
		sprite = nullptr;

	return sprite;
}

void Harvey::invokeCompletedCommand() {
	assert(_command._isComplete);
	PlayerCommand cmd = _command;
	_command = {};

	switch (cmd._action) {
	case PlayerAction::Walk: {
		auto *exit = dynamic_cast<RoomExit *>(objectById(cmd._target));
		if (exit == nullptr)
			break; // nothing to do, the player arrived at their target

		const auto scriptId = exit->scriptFor(PlayerAction::Use);
		if (scriptId == 0)
			triggerExit(exit->exitId());
		else
			script().runNew(scriptId);
		break;
	}
	case PlayerAction::Look: {
		IInteractable *object = dynamic_cast<IInteractable *>(objectById(cmd._target));
		assert(object != nullptr);
		script().runNew(object->scriptFor(PlayerAction::Look));
		break;
	}
	case PlayerAction::WhatIs: {
		Topic *topic = dynamic_cast<Topic *>(_topicRow.byId(cmd._target));
		assert(topic != nullptr);
		const auto dbRoomObject = g_engine->db().roomObject(topic->roomObjectId());
		const auto dbInteraction = g_engine->db().roomInteraction(dbRoomObject._toInteraction);
		script().runNew(dbInteraction._useScript);
		break;
	}
	case PlayerAction::TalkAbout: {
		const auto dbTopic = g_engine->db().topic(cmd._target);
		script().runNew(dbTopic._script);
		_dragStatus = DragStatus::Dropped;
		break;
	}
	default:
		assert(false && "Unhandled player action in invokeCommand");
		break;
	}
}

void Harvey::updateHover(Sprite *selection) {
	auto *interactable = dynamic_cast<IInteractable *>(selection);
	if (selection == nullptr) {
		_command._action = PlayerAction::None; // for Harvey there is are no two-click commands
		_command._target = 0;
	} else if (selection == &_buttonToEdna) {
		if (g_engine->input().isMouseLeftPressed())
			_buttonToEdna.setPressed();
		else
			_buttonToEdna.setHovered();
		_command._action = PlayerAction::ToEdna;
	} else if (dynamic_cast<RoomExit *>(interactable) != nullptr) {
		_command._target = 0;
		g_engine->assets().pushExitCursor();
	} else if (interactable != nullptr)
		_command._target = selection->id();
}

void Harvey::onMouseLeftPressed(Sprite *selection) {
	auto *exit = dynamic_cast<RoomExit *>(selection);
	if (exit == nullptr)
		_dragStatus = DragStatus::StartDrag;

	if (selection == nullptr) { // WALK TO <arbitrary point>
		_command = {};
		_command._action = PlayerAction::Walk;
		_command._targetPos = g_engine->input().mousePos();
		_command._isComplete = true;
		player().pathWalkTo(_command._targetPos);
	} else if (isTopic(selection))
		; // topic is handled by dragging
	// here would have been interaction with the "To Edna" button, but the original code is broken
	// the button only triggers on mouse release
	else if (exit != nullptr || selection->id() == _pastIds._ednaId) { 
		// Edna is special, it both starts dragging the topic "Edna"
		// and is the only object that Harvey walks to on press
		// otherwise this is WALK TO <exit>
		const auto interactable = dynamic_cast<IInteractable *>(selection);
		_command = {};
		_command._isComplete = true;
		_command._action = PlayerAction::Walk;
		_command._target = selection->id();
		_command._targetPos = interactable->interactionPos();
		if (_command._targetPos == kInvalidPoint)
			_command._targetPos = selection->pos();
		player().pathWalkTo(_command._targetPos, interactable->interactionDir());
	}
}

void Harvey::onMouseLeftReleased(Sprite *selection) {
	_dragStatus = DragStatus::StartDrop;
	if (selection == &_buttonToEdna)
		switchToEdna();
}

void Harvey::onMouseRightReleased(Sprite *selection) {
	const auto interactable = dynamic_cast<IInteractable *>(selection);
	const auto exit = dynamic_cast<RoomExit *>(selection);

	if (selection == nullptr)
		_command = {};
	else if (isTopic(selection)) {
		if (g_engine->db().topic(selection->id())._inventoryPos > 0) { // WHAT IS <topic>
			_command = {};
			_command._action = PlayerAction::WhatIs;
			_command._target = selection->id();
			_command._isComplete = true;
		} else // topics outside the topic row *should* not happen
			_command = {};
	} else if (interactable != nullptr && exit == nullptr) { // LOOK AT <object>
		_command = {};
		_command._isComplete = true;
		_command._action = PlayerAction::Look;
		_command._target = selection->id();
		_command._targetPos = interactable->interactionPos();
		if (_command._targetPos == kInvalidPoint)
			_command._targetPos = selection->pos();
		player().pathWalkTo(_command._targetPos, interactable->interactionDir());
	}
}

void Harvey::onStartDrag(Sprite *selection) {

}

void Harvey::onStartDrop(Sprite *selection) {

}

void Harvey::switchToEdna() {

}

}

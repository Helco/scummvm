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
#include "edna/game/ednagame.h"
#include "edna/sprite/character.h"
#include "edna/sprite/object.h"
#include "edna/translation.h"

using namespace Common;

namespace Edna {

static constexpr uint32 kButtonIdLook = 10;
static constexpr uint32 kButtonIdPick = 11;
static constexpr uint32 kButtonIdTalk = 12;
static constexpr uint32 kButtonIdUse = 13;

static String getButtonPath(const char *guiBasePath, const char *buttonName) {
	return String::format("gui/%s/%s/b_%s", guiBasePath, g_engine->language(), buttonName);
}

EdnaGame::EdnaGame(ScopedPtr<GameBase> &myPtr, GameMode mode, const GameTransition &transition, const char *guiBasePath)
	: Game(myPtr, mode, transition)
	, _choiceList(*this)
	, _commandPrompt(*this)
	, _buttonLook(kButtonIdLook, Point(0, 565), getButtonPath(guiBasePath, "ansehen"))
	, _buttonPick(kButtonIdPick, Point(100, 565), getButtonPath(guiBasePath, "nehmen"))
	, _buttonTalk(kButtonIdTalk, Point(200, 565), getButtonPath(guiBasePath, "reden"))
	, _buttonUse(kButtonIdUse, Point(300, 565), getButtonPath(guiBasePath, "benutzen")) {

	const auto &translation = g_engine->translation();
	_buttonLook.setDisplayName(translation.action(PlayerAction::Look));
	_buttonPick.setDisplayName(translation.action(PlayerAction::Pick));
	_buttonTalk.setDisplayName(translation.action(PlayerAction::Talk));
	_buttonUse.setDisplayName(translation.action(PlayerAction::Use));
	gui().add(&_buttonLook, DisposeAfterUse::NO);
	gui().add(&_buttonPick, DisposeAfterUse::NO);
	gui().add(&_buttonTalk, DisposeAfterUse::NO);
	gui().add(&_buttonUse, DisposeAfterUse::NO);
	texts().add(&_commandPrompt, DisposeAfterUse::NO);
}

void EdnaGame::triggerChoiceList(ChoiceSetId setId) {
	_choiceList.openSet(setId);
}

PlayerAction EdnaGame::isPlayerActionButton(Sprite *selection) const {
	if (selection == &_buttonLook)
		return PlayerAction::Look;
	if (selection == &_buttonPick)
		return PlayerAction::Pick;
	if (selection == &_buttonTalk)
		return PlayerAction::Talk;
	if (selection == &_buttonUse)
		return PlayerAction::Use;
	return PlayerAction::None;
}

void EdnaGame::updateHover(Sprite *selection) {
	Button *selectedButton = dynamic_cast<Button *>(selection);
	if (selectedButton != nullptr)
		selectedButton->setHovered();

	auto *interactable = dynamic_cast<IInteractable *>(selection);
	if (selection == nullptr) {
		_command._target = 0;
	}
	else if (dynamic_cast<RoomExit *>(interactable) != nullptr) {
		_command._target = 0;
		g_engine->assets().pushExitCursor();
	}
	else if (_command._action == PlayerAction::None) {
		const auto defaultAction = interactable == nullptr ? PlayerAction::None : interactable->defaultAction();
		switch (defaultAction) {
		case PlayerAction::Look:
			_buttonLook.setHovered();
			break;
		case PlayerAction::Pick:
			_buttonPick.setHovered();
			break;
		case PlayerAction::Talk:
			_buttonTalk.setHovered();
			break;
		case PlayerAction::Use:
			_buttonUse.setHovered();
			break;
		}
	}
	else if (interactable != nullptr)
		_command._target = selection->id();
}

void EdnaGame::invokeObjectCommand() {
	// this method only handles interaction with objects as it is called
	// after the player character walked to the object and then interacts with it
	assert(_command._isComplete);
	PlayerCommand cmd = _command;
	_command = {}; // early lest we forget to reset it

	const auto findInteractable = [&](const uint32 id) -> IInteractable &{
		assert(id != 0);
		Sprite *sprite = objectById(id);
		auto interactable = dynamic_cast<IInteractable *>(sprite);
		assert(interactable != nullptr);
		return *interactable;
		};

	switch (cmd._action) {
	case PlayerAction::Pick: // PICK <object>
	case PlayerAction::Look: // LOOK AT <object>
	case PlayerAction::Talk: // TALK WITH <object>
		script().runNew(findInteractable(cmd._target).scriptFor(cmd._action));
		break;
	case PlayerAction::Walk: { // WALK TO <object>
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
	case PlayerAction::Use:
		if (cmd._item != 0) // USE ... WITH <object> (item-item-interaction is not handled here)
			script().runNew(g_engine->db().roomItemInteraction(cmd._item, cmd._target));
		else // USE <object>
			script().runNew(findInteractable(cmd._target).scriptFor(cmd._action));
		break;
	default:
		assert(false && "Unhandled player action in invokeCommand");
		break;
	}
}

void EdnaGame::invokeItemCommand(Sprite *&selection) {
	auto item = dynamic_cast<Item *>(selection);
	assert(item != nullptr); // we technically only need IInteractable

	switch (_command._action) {
	case PlayerAction::None:
	case PlayerAction::Pick:
	case PlayerAction::Use: {
		if (_command._item == 0) { // USE <item> (also default)
			ScriptId scriptId = item->scriptFor(PlayerAction::Use);
			if (scriptId == 0) {
				_command._action = PlayerAction::Use;
				_command._item = item->id();
			}
			else {
				_command = {};
				selection = nullptr;
				script().runNew(scriptId);
			}
		}
		else { // USE <item> WITH <item>
			ScriptId scriptId = g_engine->db().itemInteraction(_command._item, _command._target);
			if (scriptId == 0)
				scriptId = g_engine->db().itemInteraction(_command._target, _command._item);
			_command = {};
			selection = nullptr;
			script().runNew(scriptId);
		}
		break;
	}
	case PlayerAction::Talk: // TALK WITH <item>
	case PlayerAction::Look: { // LOOK AT <item>
		ScriptId scriptId = item->scriptFor(_command._action);
		if (scriptId != 0)
			script().runNew(scriptId);
		_command = {};
		selection = nullptr;
		break;
	}
	}
}

void EdnaGame::invokeDefaultCommand(Sprite *selection, bool isItem) {
	auto *interactable = dynamic_cast<IInteractable *>(selection);

	if (selection == nullptr)
		_command = {};
	else if (isItem) {
		// Interact by default action with some item
		assert(interactable != nullptr);
		const auto defaultAction = interactable->defaultAction();
		ScriptId scriptId = interactable->scriptFor(defaultAction);
		if (scriptId != 0)
			script().runNew(scriptId);
		else if (defaultAction == PlayerAction::Use) {
			// or setup interaction of item with something else
			_command._action = PlayerAction::Use;
			_command._item = selection->id();
		}
	}
	else if (interactable != nullptr) {
		// Interact by default action with some object or NPC
		const auto defaultAction = interactable->defaultAction();
		if (_command._action != PlayerAction::None)
			_command = {}; // or just cancel if we were building some other command
		else if (defaultAction != PlayerAction::None)
			invokeRoomInteraction(selection, defaultAction);
	}
}


void EdnaGame::invokeRoomInteraction(Sprite *object, PlayerAction action) {
	auto *interactable = dynamic_cast<IInteractable *>(object);
	assert(object != nullptr && interactable != nullptr);
	_command._action = action;
	_command._target = object->id();
	_command._isComplete = true;
	_command._targetPos = interactable->interactionPos();
	if (_command._targetPos == kInvalidPoint)
		_command._targetPos = object->pos();
	player().pathWalkTo(_command._targetPos, interactable->interactionDir());
}

}

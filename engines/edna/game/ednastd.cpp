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
#include "edna/input.h"
#include "edna/game/ednastd.h"
#include "edna/sprite/player.h"
#include "edna/translation.h"

using namespace Common;

namespace Edna {

static constexpr uint32 kButtonIdLook = 10;
static constexpr uint32 kButtonIdPick = 11;
static constexpr uint32 kButtonIdTalk = 12;
static constexpr uint32 kButtonIdUse = 13;

EdnaStd::EdnaStd(ScopedPtr<GameBase> &myPtr, const GameTransition &transition)
	: Game(myPtr, GameMode::EdnaStd, transition)
	, _buttonLook(kButtonIdLook, Point(0, 565), String("gui/edna/") + g_engine->language() + "/b_ansehen")
	, _buttonPick(kButtonIdPick, Point(100, 565), String("gui/edna/") + g_engine->language() + "/b_nehmen")
	, _buttonTalk(kButtonIdTalk, Point(200, 565), String("gui/edna/") + g_engine->language() + "/b_reden")
	, _buttonUse(kButtonIdUse, Point(300, 565), String("gui/edna/") + g_engine->language() + "/b_benutzen")
	, _commandPrompt(*this)
	, _inventory("Inventory") {

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

bool EdnaStd::isItem(Sprite *selection) const {
	return &selection->group() == &_inventory && selection->id() > 0;
}

PlayerAction EdnaStd::isPlayerActionButton(Sprite *selection) const {
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

void EdnaStd::update() {
	Game::update();

	Sprite *selection = findSelection();
	Button *selectedButton = dynamic_cast<Button *>(selection);
	if (selectedButton != nullptr)
		selectedButton->setHovered();

	// TODO: Add inventory handling

	if (_command._isComplete) {
		if (player().state() != Character::kWalking) // TODO: This is original but should it be == kWaiting?
			invokeCommand();
	} else { // TODO: Needs condition for inventory control sprite
		updateCommandByHover(selection);
	}

	const auto &input = g_engine->input();
	if (input.wasMouseLeftPressed())
		onMouseLeftPressed(selection);
	if (input.wasMouseLeftReleased())
		onMouseLeftReleased(selection);
	if (input.wasMouseRightPressed())
		onMouseRightPressed(selection);
	if (input.wasMouseRightReleased())
		onMouseRightReleased(selection);

	_commandPrompt.setText(_command, selection);
}

Sprite *EdnaStd::findSelection() {
	const Point mousePos = g_engine->input().mousePos();
	Sprite *sprite = gui().checkClick(mousePos);
	// TODO: Comment group is missing
	if (sprite == nullptr)
		sprite = objects().checkInteractableClick(mousePos);
	if (sprite == nullptr)
		sprite = bgObjects().checkClick(mousePos);
	return sprite;
}

void EdnaStd::updateCommandByHover(Sprite *selection) {
	auto *interactable = dynamic_cast<IInteractableObject *>(selection);

	if (selection == nullptr) {
		_command._target = 0;
	} else if (_command._action == PlayerAction::None) {
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
	} else if (interactable != nullptr) {
		_command._target = interactable->id();
	}
	// The other cases 
}

void EdnaStd::onMouseLeftPressed(Sprite *selection) {
	if (selection == nullptr) {
		// TODO: Check that inventory is closed
		// TODO: Use path finding
		_command = {};
		_command._action = PlayerAction::Walk;
		_command._targetPos = g_engine->input().mousePos();
		_command._isComplete = true;
		player().walkTo(_command._targetPos);
		return;
	}
	// TODO: Filter interaction with comment or achievement close button

	auto *interactable = dynamic_cast<IInteractableObject *>(selection);
	auto playerAction = isPlayerActionButton(selection);
	if (isItem(selection)) {
		assert(interactable != nullptr);
		switch (_command._action) {
		case PlayerAction::None:
		case PlayerAction::Pick:
		case PlayerAction::Use: {
			ScriptId scriptId = interactable->scriptFor(PlayerAction::Use);
			if (scriptId == 0) { // TODO: Recheck triggering of script 0 for interactions
				_command._action = PlayerAction::Use;
				_command._item = interactable->id();
			} else {
				_command = {};
				script().runNew(scriptId);
			}
			break;
		}
		case PlayerAction::Talk:
		case PlayerAction::Look: {
			ScriptId scriptId = interactable->scriptFor(_command._action);
			if (scriptId != 0)
				script().runNew(scriptId);
			_command = {};
			break;
		}
		case PlayerAction::Walk:
			break;
		default:
			// TODO: Too bad this is wrong -_-
			break;
		}
	} else if (playerAction != PlayerAction::None) {
		_command = {};
		_command._action = playerAction;
	} else if (interactable != nullptr) { // TODO: Add Inventory controls here
		if (_command._action == PlayerAction::None)
			_command._action = PlayerAction::Walk;
		// TODO: Add weird exit condition here
		invokeRoomInteraction(interactable, _command._action);
	}
}

void EdnaStd::onMouseLeftReleased(Sprite *selection) {
	auto playerAction = isPlayerActionButton(selection);
	if (playerAction != PlayerAction::None)
		_command._action = playerAction;
}

void EdnaStd::onMouseRightPressed(Sprite *selection) {
	if (selection == nullptr)
		_command = {};
}

void EdnaStd::onMouseRightReleased(Sprite *selection) {
	auto *interactable = dynamic_cast<IInteractableObject *>(selection);

	if (selection == nullptr)
		_command = {};
	else if (isItem(selection)) {
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
	} else if (interactable != nullptr) {
		// Interact by default action with some object or NPC
		const auto defaultAction = interactable->defaultAction();
		if (_command._action != PlayerAction::None)
			_command = {}; // or just cancel if we were building some other command
		else if (defaultAction != PlayerAction::None)
			invokeRoomInteraction(interactable, defaultAction);
	}
}

void EdnaStd::invokeRoomInteraction(IInteractableObject *object, PlayerAction action) {
	_command._action = action;
	_command._target = object->id();
	_command._isComplete = true;
	_command._targetPos = object->interactionPos();
	if (_command._targetPos == kInvalidPoint)
		_command._targetPos = object->pos();
	// TODO: Use path finding
	player().walkTo(_command._targetPos, object->interactionDir());
}

void EdnaStd::invokeCommand() {
	assert(_command._isComplete);
	warning("Command invocation is not implemented yet");
}

}

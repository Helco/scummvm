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
#include "edna/sprite/character.h"

using namespace Common;

namespace Edna {

EdnaStd::EdnaStd(ScopedPtr<GameBase> &myPtr, const GameTransition &transition)
	: EdnaGame(myPtr, GameMode::EdnaStd, transition, "edna") {

	init(transition);
}

void EdnaStd::initGroups() {
	Game::initGroups(&_inventory, &_choiceList);
}

bool EdnaStd::isItem(Sprite *selection) const {
	return selection != nullptr && &selection->group() == &_inventory && selection->id() > 0;
}

void EdnaStd::update() {
	Game::update();
	_inventory.active() = gui().active();

	if (script().isScriptRunning() || script().isPerforming() || _choiceList.active()) {
		_inventory.close();
		return;
	}

	Sprite *selection = findSelection();
	if (dynamic_cast<RoomExit *>(selection) != nullptr)
		g_engine->setExitCursor();

	if (_command._isComplete) {
		if (player().state() == Character::kWaiting) {
			selection = nullptr;
			invokeObjectCommand();
		}
	} else {
		updateHover(selection, true);
		_inventory.updateSelection(selection);
	}

	const auto &input = g_engine->input();
	if (input.wasMouseLeftPressed())
		onMouseLeftPressed(selection);
	if (input.wasMouseLeftReleased())
		onMouseLeftReleased(selection);
	if (input.wasMouseRightPressed())
		onMouseRightPressed(selection);
	if (input.wasMouseRightReleased())
		invokeDefaultCommand(selection, isItem(selection));

	_commandPrompt.setText(_command, selection);
}

Sprite *EdnaStd::findSelection() {
	const Point mousePos = g_engine->input().mousePos();
	Sprite *sprite = gui().checkClick(mousePos);
	// TODO: Comment group is missing
	if (sprite == nullptr)
		sprite = _inventory.checkClick(mousePos);
	if (sprite == nullptr)
		sprite = objects().checkInteractableClick(mousePos);
	if (sprite == nullptr)
		sprite = bgObjects().checkClick(mousePos);
	return sprite;
}

void EdnaStd::onMouseLeftPressed(Sprite *&selection) {
	if (selection == nullptr) {
		if (_inventory.isClosed()) {
			_command = {};
			_command._action = PlayerAction::Walk;
			_command._targetPos = g_engine->input().mousePos();
			_command._isComplete = true;
			player().pathWalkTo(_command._targetPos);
		}
		return;
	}
	// TODO: Filter interaction with comment or achievement close button

	auto *interactable = dynamic_cast<IInteractable *>(selection);
	auto playerAction = isPlayerActionButton(selection);
	if (isItem(selection))
		invokeItemCommand(selection);
	else if (playerAction != PlayerAction::None) {
		_command = {};
		_command._action = playerAction;
	} else if (_inventory.updatePressed(selection))
		return;
	else if (interactable != nullptr) {
		if (_command._action == PlayerAction::None)
			_command._action = PlayerAction::Walk;
		if (dynamic_cast<RoomExit *>(interactable) == nullptr || _command._action == PlayerAction::Walk)
			invokeRoomInteraction(selection, _command._action);
		// Room exits do not react at all to other commands
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

void EdnaStd::triggerInventoryUpdate() {
	_inventory.onItemsChanged();
}

}

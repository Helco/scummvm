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
#include "edna/game/scriptonclick.h"
#include "edna/input.h"
#include "edna/sprite/character.h"
#include "edna/sprite/object.h"

using namespace Common;

namespace Edna {

ScriptOnClick::ScriptOnClick(ScopedPtr<GameBase> &myPtr, const GameTransition &transition)
	: Game(myPtr, GameMode::ScriptOnClick, transition)
	, _commandPrompt(*this)
	, _choiceList(*this) {

	texts().add(&_commandPrompt, DisposeAfterUse::NO);

	init(transition);
}

void ScriptOnClick::initGroups() {
	Game::initGroups(nullptr, &_choiceList);
}

void ScriptOnClick::update() {
	Game::update();

	if (script().isScriptRunning() || script().isPerforming() || _choiceList.active())
		return;

	Sprite *selection = findSelection();
	if (dynamic_cast<RoomExit *>(selection) != nullptr)
		g_engine->setExitCursor();

	if (_command._isComplete) {
		if (player().state() == Character::kWaiting) {
			selection = nullptr;
			invokeObjectCommand();
		}
	} else {
		if (selection == nullptr || dynamic_cast<RoomExit *>(selection) != nullptr)
			_command._target = 0;
		else if (dynamic_cast<IInteractable *>(selection) != nullptr)
			_command._target = selection->id();
	}

	const auto &input = g_engine->input();
	if (input.wasMouseLeftPressed())
		onMouseLeftPressed(selection);
	if (input.wasMouseRightPressed())
		onMouseRightPressed(selection);
	if (input.wasMouseRightReleased())
		invokeDefaultCommand(selection, false);

	_commandPrompt.setText(_command, selection);
}

void ScriptOnClick::triggerChoiceList(ChoiceSetId setId) {
	_choiceList.openSet(setId);
}

Sprite *ScriptOnClick::findSelection() {
	const Point mousePos = g_engine->input().mousePos();
	Sprite *sprite = gui().checkClick(mousePos);
	if (sprite == nullptr)
		sprite = objects().checkInteractableClick(mousePos);
	if (sprite == nullptr)
		sprite = bgObjects().checkClick(mousePos);
	return sprite;
}

void ScriptOnClick::onMouseLeftPressed(Sprite *&selection) {
	if (dynamic_cast<IInteractable *>(selection) != nullptr)
		invokeRoomInteraction(selection, PlayerAction::Walk);
}

void ScriptOnClick::onMouseRightPressed(Sprite *selection) {
	if (selection == nullptr)
		_command = {};
}

}

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

#include "edna/db.h"
#include "edna/edna.h"
#include "edna/input.h"
#include "edna/game/ednagirl.h"
#include "edna/sprite/character.h"
#include "edna/translation.h"

using namespace Common;

namespace Edna {

static constexpr uint32 kButtonIdToHarvey = 14;

EdnaGirl::EdnaGirl(ScopedPtr<GameBase> &myPtr, const GameTransition &transition)
	: EdnaGame(myPtr, GameMode::EdnaGirl, transition, "ednajung")
	, _buttonToHarvey(kButtonIdToHarvey, Point(698, 525), "gui/ednajung/b_zuharvey") {

	const auto &translation = g_engine->translation();
	_buttonToHarvey.setDisplayName(translation.action(PlayerAction::ToHarvey));
	gui().add(&_buttonToHarvey, DisposeAfterUse::NO);

	init(transition);
}

void EdnaGirl::initGroups() {
	Game::initGroups(nullptr, &_choiceList);
}

void EdnaGirl::update() {
	Game::update();

	if (script().isScriptRunning() || script().isPerforming() || _choiceList.active())
		return;

	Sprite *selection = findSelection();

	if (_command._isComplete) {
		if (player().state() == Character::kWaiting) {
			selection = nullptr;
			invokeObjectCommand();
		}
	}
	else {
		updateHover(selection);
		// TODO: _inventory.updateSelection(selection);
	}

	const auto &input = g_engine->input();
	/*if (input.wasMouseLeftPressed())
		onMouseLeftPressed(selection);
	if (input.wasMouseLeftReleased())
		onMouseLeftReleased(selection);
	if (input.wasMouseRightPressed())
		onMouseRightPressed(selection);*/
	if (input.wasMouseRightReleased())
		invokeDefaultCommand(selection, isItem(selection));

}

void EdnaGirl::triggerInventoryUpdate() {
	// TODO
}

bool EdnaGirl::isItem(Sprite *selection) const {
	// TODO
	return false;
}

Sprite *EdnaGirl::findSelection() {
	const Point mousePos = g_engine->input().mousePos();
	Sprite *sprite = gui().checkClick(mousePos);
	// TODO: Comment group is missing
	// TODO: if (sprite == nullptr)
	//	sprite = _inventory.checkClick(mousePos);
	if (sprite == nullptr)
		sprite = objects().checkInteractableClick(mousePos);
	if (sprite == nullptr)
		sprite = bgObjects().checkClick(mousePos);
	return sprite;
}

}

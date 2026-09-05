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

void EdnaGame::updateHover(Sprite *selection, bool selectDefaultAction) {
	Button *selectedButton = dynamic_cast<Button *>(selection);
	if (selectedButton != nullptr) {
		if (g_engine->input().isMouseLeftPressed())
			selectedButton->setPressed();
		else
			selectedButton->setHovered();
	}

	auto *interactable = dynamic_cast<IInteractable *>(selection);
	if (selection == nullptr) {
		_command._target = 0;
	}
	else if (dynamic_cast<RoomExit *>(selection) != nullptr)
		_command._target = 0;
	else if (_command._action == PlayerAction::None) {
		const auto defaultAction = selectDefaultAction && interactable != nullptr
			? interactable->defaultAction()
			: PlayerAction::None;
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

}

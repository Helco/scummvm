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
#include "edna/game/game.h"
#include "edna/sprite/commandprompt.h"
#include "edna/translation.h"

#include "gui/debugger.h"

using namespace Common;

namespace Edna {

CommandPrompt::CommandPrompt(Game &game)
	: _game(game)
	, _inactiveFont(g_engine->assets().font(FontKind::InactiveFont))
	, _activeFont(g_engine->assets().font(FontKind::ActiveFont))
	, _rendered(g_engine->renderer().createText(_inactiveFont)) {
	pos() = Point(10, 530);
}

void CommandPrompt::render() {
	g_engine->renderer().text(_rendered.get(), pos());
}

void CommandPrompt::debugPrint() {
	g_engine->getDebugger()->debugPrintf("Command prompt");
}

void CommandPrompt::setText(const PlayerCommand &command, Sprite *selection) {
	uint32 selectionId = selection == nullptr ? UINT32_MAX : selection->id();
	if (command == _lastCommand && selectionId == _lastSelectionId)
		return;
	_lastCommand = command;
	_lastSelectionId = selectionId;
	_text.assign(0, ' '); // clear would free, this keeps the allocation

	if (command._action == PlayerAction::None) {
		if (selection != nullptr)
			_text = selection->displayName();
	} else {
		const auto &translation = g_engine->translation();
		const auto &db = g_engine->db();

		_text += translation.action(command._action);
		if (command._item != 0) {
			_text += ' ';
			_text += db.item(command._item)._name;
			_text += ' ';
			_text += translation.actionWith();
		}
		if (command._target != 0) {
			const auto dbObject = db.roomObject(command._target, false);
			const char *targetName = dbObject._name;
			if (dbObject._toInteraction != 0)
				targetName = db.roomInteraction(dbObject._toInteraction)._name;
			if (targetName == nullptr)
				targetName = db.item(command._target, false)._name;
			if (targetName == nullptr)
				targetName = db.topic(command._target, false)._name;
			if (targetName == nullptr) {
				targetName = "<unknown>";
				warning("Unknown target ID: %u", command._target);
			}
			_text += ' ';
			_text += targetName;
		}
	}

	_rendered->setText(_text.c_str());
	_rendered->setColor(command._isComplete ? _activeFont : _inactiveFont);
	size() = _rendered->size();
}

}

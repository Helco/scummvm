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
#include "edna/game/game.h"
#include "edna/group/choicelist.h"
#include "edna/sprite/text.h"

using namespace Common;

namespace Edna {

ChoiceList::ChoiceList(Game &game)
	: Group("Choices")
	, _game(game) {
	active() = false;
}

void ChoiceList::update() {
	if (!_active)
		return;
	assert(_setId != 0);
	Group::update();

	if (_selected != nullptr)
		_selected->setColor(FontKind::InactiveFont);
	_selected = dynamic_cast<Text *>(checkClick(g_engine->input().mousePos()));
	if (_selected == nullptr)
		return;

	_selected->setColor(FontKind::ActiveFont);
	if (g_engine->input().isMouseLeftPressed()) {
		const auto dbChoice = g_engine->db().choice(_setId, _selected->id());
		if (dbChoice._script != 0) {
			close();
			_game.script().runNew(dbChoice._script);
		}
	}
}

void ChoiceList::openSet(ChoiceSetId setId) {
	constexpr Point kOrigin = Point(18, 333);
	constexpr Point kLineSize = Point(0, 25);

	assert(setId != 0);
	close();
	_setId = setId;
	const auto dbChoices = g_engine->db().choices(setId);
	int count = 0;
	for (const auto &dbChoice : dbChoices) {
		if (dbChoice._active)
			count++;
	}

	Sprite *frame = new Sprite();
	frame->setTexture(String::format("gui/edna/bg_choicelist_%d.png", count).c_str());
	frame->pos() = Point(0, 313);
	add(frame, DisposeAfterUse::YES);

	for (const auto &dbChoice : dbChoices) {
		if (!dbChoice._active)
			continue;

		Text *line = new Text(kOrigin + kLineSize * ((int)_sprites.size() - 1 + 10 - count),
			FontKind::InactiveFont, dbChoice._text, {});
		line->id() = dbChoice._line;
		add(line, DisposeAfterUse::YES);
	}
	active() = true;
	_game.gui().active() = false;
}

void ChoiceList::close() {
	_setId = 0;
	_selected = nullptr;
	_sprites.clear();
	active() = false;
	_game.gui().active() = true;
}

}

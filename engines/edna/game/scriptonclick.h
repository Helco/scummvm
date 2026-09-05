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

#ifndef EDNA_SCRIPTONCLICK_H
#define EDNA_SCRIPTONCLICK_H

#include "edna/game/game.h"
#include "edna/group/choicelist.h"
#include "edna/sprite/commandprompt.h"

namespace Edna {

class ScriptOnClick : public Game {
public:
	ScriptOnClick(Common::ScopedPtr<GameBase> &myPtr, const GameTransition &transition);

	void initGroups() override;
	void update() override;
	void triggerChoiceList(ChoiceSetId setId) override;

private:
	Sprite *findSelection();
	void onMouseLeftPressed(Sprite *&selection);
	void onMouseRightPressed(Sprite *selection);

	CommandPrompt _commandPrompt;
	ChoiceList _choiceList;
};

}

#endif // EDNA_SCRIPTONCLICK_H

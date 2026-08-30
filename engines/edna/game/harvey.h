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

#ifndef EDNA_HARVEY_H
#define EDNA_HARVEY_H

#include "edna/game/game.h"
#include "edna/group/choicelist.h"
#include "edna/group/topicrow.h"
#include "edna/sprite/button.h"
#include "edna/sprite/commandprompt.h"

namespace Edna {

class Harvey final : Game {
public:
	Harvey(Common::ScopedPtr<GameBase> &myPtr, const GameTransition &transition);

	void update() override;
	void triggerChoiceList(ChoiceSetId setId) override;

protected:
	void initGroups() override;

	bool isTopic(Sprite *sprite) const;
	bool isTopicRow(Sprite *sprite) const;
	TopicId isTopicObject(Sprite *sprite);
	Sprite *findSelection();
	void invokeCompletedCommand();
	void updateHover(Sprite *selection);
	void onMouseLeftPressed(Sprite *selection);
	void onMouseLeftReleased(Sprite *selection);
	void onMouseRightReleased(Sprite *selection);
	void onStartDrag(Sprite *selection);
	void onStartDrop(Sprite *selection);

	const PastRoomIds _pastIds;
	PlayerCommand _command = {};
	CommandPrompt _commandPrompt;
	ChoiceList _choiceList;
	TopicRow _topicRow;
	Button _buttonToEdna;
	Sprite _inactiveButtonToEdna;
	DragStatus _dragStatus = DragStatus::Dropped;
	Topic *_dragTopic = nullptr;
};

}

#endif // EDNA_HARVEY_H

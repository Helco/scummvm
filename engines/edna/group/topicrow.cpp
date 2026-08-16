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
#include "edna/group/topicrow.h"
#include "edna/sprite/object.h"
#include "edna/translation.h"

using namespace Common;

namespace Edna {

static Point topicPositionAt(uint slotI) {
	return Point(50 * slotI - 47, 555);
}

TopicRow::TopicRow(RoomId roomId)
	: Group("Topics")
	, _frame(g_engine->translation().dropTopic()) {

	_frame.immutable() = true;
	_frame.pos() = Point(0, 551);
	_frame.setTexture("gui/harvey/topicleiste.png");
	add(&_frame, DisposeAfterUse::NO);

	const auto dbTopics = g_engine->db().topicsByChapter(roomId);
	for (const auto topicId : dbTopics) {
		const auto dbTopic = g_engine->db().topic(topicId);
		Topic *topic = new Topic(topicId, dbTopic._roomObject);
		topic->immutable() = true;
		topic->pos() = topicPositionAt(dbTopic._topicRowPos);
		topic->toggle(dbTopic._topicRowPos > 0);
		add(topic, DisposeAfterUse::YES);
	}
	sortTopics();
}

static constexpr uint kMaxSlots = 14;
void TopicRow::moveTopic(Topic *topic, uint slotI) {
	assert(topic != nullptr && &topic->group() == this);
	if (slotI > kMaxSlots)
		return;
	if (topic->active() && slotI == 0) {
		topic->toggle(false);
		g_engine->db().setTopicPos(topic->id(), 0);
		return;
	}

	for (uint i = 0; i < kMaxSlots; i++) {
		uint newSlotI = (slotI - 1 + i) % kMaxSlots + 1;
		const auto itOtherSprite = find_if(_sprites.begin(), _sprites.end(), [&](const DisposablePtr<Sprite> &sprite) {
			const auto dbTopic = g_engine->db().topic(sprite->id(), false);
			return dbTopic._topicRowPos == newSlotI;
		});
		if (itOtherSprite == _sprites.end()) {
			// this slot is free
			topic->pos() = topicPositionAt(newSlotI);
			topic->toggle(true);
			g_engine->db().setTopicPos(topic->id(), newSlotI);
			sortTopics();
			return;
		} else if (itOtherSprite->get() == topic)
			return; // topic is already at suitable position
	}

	// nothing happens if the topic row is full, this should not happen in practice
}

void TopicRow::sortTopics() {
	stable_sort(_sprites.begin(), _sprites.end(), [](const DisposablePtr<Sprite> &a, const DisposablePtr<Sprite> &b) {
		const auto dbTopicA = g_engine->db().topic(a->id(), false);
		const auto dbTopicB = g_engine->db().topic(b->id(), false);
		return dbTopicA._topicRowPos <= dbTopicB._topicRowPos;
	});
}

}

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
#include "edna/group/inventory.h"
#include "edna/sprite/object.h"

using namespace Common;

namespace Edna {

Inventory::Inventory(GameMode gameMode)
	: Group("Inventory")
	, _gameMode(gameMode) {}

void Inventory::updateSelection(Sprite *selection) {
	if (selection != nullptr && &selection->group() == this && selection->id() > 0)
		dynamic_cast<Item *>(selection)->setHovered();
}

void Inventory::onItemsChanged() {
	updateItems();
}

void Inventory::updateItems() {
	if (_firstItemI == UINT_MAX)
		_firstItemI = _sprites.size();

	// both these spans are sorted by position so we do not reload sprites we already have
	auto dbItems = g_engine->db().ownedItems(_gameMode);
	uint spriteI = _firstItemI;

	for (uint slotI = 0; slotI < dbItems.size(); slotI++, spriteI++) {
		const auto dbItem = g_engine->db().item(dbItems[slotI]);

		// find or create new item sprite in right section
		uint readSpriteI;
		auto itReadSprite = find_if(_sprites.begin() + spriteI, _sprites.end(),
			[&](const DisposablePtr<Sprite> &sprite) { return sprite->id() == dbItem._id; });
		if (itReadSprite == _sprites.end()) {
			Item *item = new Item(dbItem._id);
			item->immutable() = true;
			add(item, DisposeAfterUse::YES);
			readSpriteI = _sprites.size() - 1;
		}
		else
			readSpriteI = itReadSprite - _sprites.begin();

		// retrieve item and update position
		if (spriteI != readSpriteI)
			SWAP(_sprites[spriteI], _sprites[readSpriteI]);
		Item *item = dynamic_cast<Item *>(_sprites[spriteI].get());
		assert(item != nullptr);
		updateItem(item, slotI);
	}

	// the right section is now only items we no longer own
	_sprites.erase(_sprites.begin() + spriteI, _sprites.end());
}

StdInventory::StdInventory()
	: Inventory(GameMode::StartMenu) // an original quirk, this is used instead of EdnaStd
	, _buttonLock(0, Point(755, 568), "gui/edna/b_schloss")
	, _buttonUnlock(0, Point(755, 568), "gui/edna/b_keinschloss")
	, _buttonUp(0, Point(774, 53), "gui/edna/b_up")
	, _buttonDown(0, Point(774, 56), "gui/edna/b_down")
{
	_buttonLock.immutable() = true;
	_buttonUnlock.immutable() = true;
	_buttonUp.immutable() = true;
	_buttonDown.immutable() = true;
	_buttonUnlock.toggle(false);
	_buttonUp.toggle(false);
	_buttonDown.toggle(false);

	const auto path = String("gui/edna/") + g_engine->language();
	_frame.setTextures({
		path + "/b_inventar.png",
		path + "/b_inventar_a.png",
		path + "/b_inventar_p.png",
		path + "/gui_inventarani_1.png",
		path + "/gui_inventarani_2.png",
		path + "/gui_inventarani_3.png",
		path + "/gui_inventarani_4.png",
		path + "/gui_inventarani_5.png",
		path + "/gui_inventarani_6.png",
		path + "/inventar_offen.png"
	});
	_frame.immutable() = true;
	_frame.setFrame(0);
	_frame.pos() = Point(400, 0);

	add(&_frame, DisposeAfterUse::NO); // add frame first so buttons take precedence for selection
	add(&_buttonLock, DisposeAfterUse::NO);
	add(&_buttonUnlock, DisposeAfterUse::NO);
	add(&_buttonUp, DisposeAfterUse::NO);
	add(&_buttonDown, DisposeAfterUse::NO);

	updateItems();
	toggleAllItems(false);
}

void StdInventory::update() {
	Group::update();

	const Point mousePos = g_engine->input().mousePos();
	if (_state == State::Open && mousePos.x < 390 && mousePos.y < 560)
		close();

	if (_state == State::Opening && !_frame.isAnimating()) {
		_state = State::Open;
		_scroll = 0;
		updateItems();
	}
}

void StdInventory::updateSelection(Sprite *selection) {
	if (selection == &_frame && _state == State::Closed) {
		_state = State::Opening;
		_frame.setAnimation(AnimationRange(0, _frame.textureCount() - 1, 60, false));
	} else
		Inventory::updateSelection(selection);
}

bool StdInventory::updatePressed(Sprite *selection) {
	if (selection == &_buttonUp) {
		_scroll++;
		updateItems();
	} else if (selection == &_buttonDown) {
		assert(_scroll > 0);
		_scroll--;
		updateItems();
	} else if (selection == &_buttonLock) {
		_buttonLock.toggle(false);
		_buttonUnlock.toggle(true);
		close();
		_state = State::Locked;
	} else if (selection == &_buttonUnlock) {
		_buttonLock.toggle(true);
		_buttonUnlock.toggle(false);
		_frame.setAnimation(AnimationRange(0, _frame.textureCount() - 1, 60, false));
		_state = State::Opening;
	} else
		return false;
	return true;
}

void StdInventory::onItemsChanged() {
	_scroll = 0;
	if (_state == State::Open)
		updateItems();
}

void StdInventory::close() {
	if (_state == State::Closed)
		return;
	_state = State::Closed;
	toggleAllItems(false);
	_buttonUp.toggle(false);
	_buttonDown.toggle(false);
	_frame.setFrame(0);
}

void StdInventory::toggleAllItems(bool active) {
	for (uint i = kFirstItemI; i < _sprites.size(); i++)
		_sprites[i]->toggle(active);
}

static constexpr Point kStdOrigin(401, 523);
static constexpr int16 kStdSlotWidth = 75;
static constexpr int16 kStdSlotHeight = -78;
static constexpr uint kStdSlotsPerLine = 5;
static constexpr int16 kStdLineCount = 7;

void StdInventory::updateItems() {
	auto dbItems = g_engine->db().ownedItems(GameMode::StartMenu);
	uint maxScroll = (dbItems.size() + kStdSlotsPerLine - 1) / kStdSlotsPerLine;
	maxScroll = maxScroll < kStdLineCount ? 0 : maxScroll - kStdLineCount;
	_scroll = MIN(_scroll, maxScroll);
	_buttonUp.toggle(_scroll < maxScroll);
	_buttonDown.toggle(_scroll > 0);

	Inventory::updateItems();
}

void StdInventory::updateItem(Item *item, uint slotI) {
	int16 line = (int16)(slotI / kStdSlotsPerLine) - _scroll;
	item->pos() = kStdOrigin + Point((slotI % kStdSlotsPerLine) * kStdSlotWidth, line * kStdSlotHeight);
	item->toggle(line >= 0 && line < kStdLineCount);
}

GirlInventory::GirlInventory(): Inventory(GameMode::EdnaGirl) {
	_frame.setTexture("gui/ednajung/itemleiste.png");
	_frame.pos() = Point(398, 551);
	add(&_frame, DisposeAfterUse::NO);

	updateItems();
}

static constexpr Point kGirlOrigin(404, 555);
static constexpr Point kGirlSlotSize(50, 0);

void GirlInventory::updateItem(Item *item, uint slotI) {
	item->pos() = kGirlOrigin + kGirlSlotSize * (int)slotI;
}

}

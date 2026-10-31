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
#include "edna/edna.h"
#include "edna/game/game.h"
#include "edna/group/savemenu.h"
#include "edna/input.h"
#include "edna/translation.h"

#include "common/savefile.h"
#include "gui/message.h"

using namespace Common;

namespace Edna {

static constexpr const Point kSlotSize(130, 100);
static constexpr const Point kSlotImageOffset(9, 8);
static constexpr const Point kSaveImageSize(112, 84);
static constexpr const uint32 kFirstSlotButtonId = 10;

static TexturePtr loadSaveThumbnail(ScopedPtr<InSaveFile> saveFile) {
	if (!saveFile)
		return TexturePtr();

	ExtendedSavegameHeader header;
	if (!g_engine->getMetaEngine()->readSavegameHeader(saveFile.get(), &header, false) ||
		header.thumbnail == nullptr) // without thumbnail we can still hide the "empty" text
		return TexturePtr();

	Graphics::ManagedSurface converted;
	converted.convertFrom(*header.thumbnail, g_system->getScreenFormat());
	header.thumbnail->free();

	ScopedPtr<Graphics::ManagedSurface> scaled(converted.scale(kSaveImageSize.x, kSaveImageSize.y, true));
	return g_engine->renderer().loadTexture(scaled->rawSurface());
}

SaveMenu::SaveMenu(bool isForLoading, Point gridPos)
	: Group(isForLoading ? "LoadMenu" : "SaveMenu")
	, _btnConfirm(1, Point(), "gui/hauptmenue/b_okay")
	, _btnDelete(2, Point(), "gui/hauptmenue/b_delete")
	, _btnCancel(3, Point(), "gui/hauptmenue/b_chancel")
	, _infoText(Point(), FontKind::MenuFont, U32String(), kTextNone)
	, _emptySlotImage(g_engine->assets().texture(
		String::format("gui/slot_empty_%s.png", g_engine->language())))
	, _isForLoading(isForLoading) {

	for (int y = 0; y < kSlotColCount; y++) {
		for (int x = 0; x < kSlotColCount; x++) {
			const int i = y * kSlotColCount + x;
			const Point pos = gridPos + Point(x * kSlotSize.x, y * kSlotSize.y);
			_btnSlots[i].reset(new Button(kFirstSlotButtonId + i, pos, "gui/hauptmenue/slot_empty"));
			_slotImages[i].id() = 20 + i;
			_slotImages[i].pos() = pos + kSlotImageOffset;
			_slotImages[i].setTexture(_emptySlotImage);
			_slotImages[i].clickable() = false;

			add(_btnSlots[i].get(), DisposeAfterUse::NO);
			add(&_slotImages[i], DisposeAfterUse::NO);
		}
	}

	add(&_btnConfirm, DisposeAfterUse::NO);
	add(&_btnDelete, DisposeAfterUse::NO);
	add(&_btnCancel, DisposeAfterUse::NO);
	
}

void SaveMenu::addRemainingSprites() {
	add(&_infoText, DisposeAfterUse::NO);
}

InGameSaveMenu::InGameSaveMenu(bool isForLoading)
	: SaveMenu(isForLoading, Point(220, 150)) {
	addRemainingSprites();

	_btnConfirm.pos() = Point(524, 450);
	_btnDelete.pos() = Point(552, 450);
	_btnCancel.pos() = Point(580, 450);
	_infoText.setPos(Point(235, 453));
}

StartSaveMenu::StartSaveMenu()
	: SaveMenu(true, Point(120, 230))
	, _infoTextBg("InfoTextBg") {

	_infoTextBg.setTexture("gui/hauptmenue/infoleiste380.png");
	_infoTextBg.pos() = Point(125, 200);
	_infoTextBg.immutable() = true;
	add(&_infoTextBg, DisposeAfterUse::NO);
	addRemainingSprites();

	_btnCancel.pos() = Point(510, 230);
	_btnConfirm.pos() = Point(510, 260);
	_btnDelete.pos() = Point(510, 290);
	_infoText.setPos(Point(135, 203));
}

void SaveMenu::open() {
	for (int i = 0; i < kSlotCount; i++) {
		_slotImages[i].setTexture(_emptySlotImage);
		_isAvailable[i] = false;
	}

	auto *saveFileMgr = g_system->getSavefileManager();
	const auto saveFiles = saveFileMgr->listSavefiles(
		g_engine->getMetaEngine()->getSavegameFilePattern());
	for (const auto &saveFileName : saveFiles) {
		if (saveFileName.size() < 3)
			continue; // somehow not adhering to the pattern
		char *slotEnd = nullptr;
		int32 slotI = (int32)strtol(saveFileName.end() - 3, &slotEnd, 10);
		if (slotEnd != saveFileName.end() || slotI < 0 || slotI >= kSlotCount)
			continue;

		ScopedPtr<InSaveFile> saveFile(saveFileMgr->openForLoading(saveFileName));
		_isAvailable[slotI] = saveFile != nullptr;
		_slotImages[slotI].setTexture(loadSaveThumbnail(move(saveFile)));
	}

	_selectedI = -1;
	_btnConfirm.toggle(false);
	_btnDelete.toggle(false);
	_infoText.setText(U32String());
	active() = true;
}

void SaveMenu::update() {
	if (!active())
		return;
	Group::update();

	const bool clicked = g_engine->input().wasMouseLeftPressed();
	Sprite *selection = checkClick(g_engine->input().mousePos());
	Button *button = dynamic_cast<Button *>(selection);
	if (button != nullptr)
		button->setHovered();
	if (clicked && selection != nullptr) {
		if (selection == &_btnCancel) {
			active() = false;
		} else if (selection == &_btnConfirm) {
			assert(_selectedI >= 0);
			if (_isForLoading)
				g_engine->loadGameState(_selectedI);
			else
				g_engine->saveGameState(_selectedI, "");
		} else if (selection == &_btnDelete) {
			assert(_selectedI >= 0);

		} else if (selection->id() >= kFirstSlotButtonId && selection->id() < kFirstSlotButtonId + kSlotCount) {
			changeSelection(selection->id() - kFirstSlotButtonId);
		} else if (_infoText.bounds().contains(g_engine->input().mousePos())) {
			U32String joke = g_engine->translation().infoJoke(_infoJokeIndex++);
			if (joke.empty()) {
				_infoJokeIndex = 0;
				joke = g_engine->translation().infoJoke(_infoJokeIndex);
			}
			assert(!joke.empty());
			_infoText.setText(joke);
		}
	}
}

void SaveMenu::changeSelection(uint32 slotI) {
	assert(slotI < (uint32)kSlotCount);
	const U32String pattern = _isForLoading ? _isAvailable[slotI]
		? g_engine->translation().infoLoadSlot()
		: g_engine->translation().infoLoadEmptySlot()
		: g_engine->translation().infoSaveSlot();
	_infoText.setText(U32String::format(pattern, (int)(slotI + 1)));

	_btnConfirm.toggle(!_isForLoading || _isAvailable[slotI]);
	_btnDelete.toggle(_isAvailable[slotI]);
	_selectedI = (int)slotI;
}

}

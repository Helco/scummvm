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

#include "gui/gui-manager.h"
#include "HatchSequenceDialog.h"

namespace TopGun {

static Common::Rect scaleRect(int x, int y, int w, int h, float scale) {
	auto rect = Common::Rect((int)(w * scale), (int)(h * scale));
	rect.translate((int)(x * scale), (int)(y * scale));
	return rect;
}

HatchSequenceDialog::HatchSequenceDialog(TamaPlugin *tamaPlugin, const Common::String &nick, const Common::String &name)
	: GUI::Dialog(0, 0, 158, 106), _tamaPlugin(tamaPlugin) {

	auto tamaResources = tamaPlugin->_tamaResources.get();
	float scale = GUI::GuiManager::instance().getFontHeight() / 8.0f;
	_w = (uint16)(_w * scale);
	_h = (uint16)(_h * scale);

	const auto nickString = tamaResources->loadString(8198);
	auto rect = scaleRect(7, 7, 142, 20, scale);
	new GUI::StaticTextWidget(this, rect.left, rect.top, rect.width(), rect.height(), nickString, Graphics::kTextAlignLeft);

	const auto nameString = tamaResources->loadString(8199);
	rect = scaleRect(7, 46, 142, 20, scale);
	new GUI::StaticTextWidget(this, rect.left, rect.top, rect.width(), rect.height(), nameString, Graphics::kTextAlignLeft);

	auto okString = tamaResources->loadString(8192);
	TamaPlugin::removeWinAPIHotkey(okString);
	rect = scaleRect(19, 85, 50, 14, scale);
	(new GUI::ButtonWidget(this, rect.left, rect.top, rect.width(), rect.height(), okString))->setCmd(kCmdOK);

	auto cancelString = tamaResources->loadString(8194);
	TamaPlugin::removeWinAPIHotkey(cancelString);
	rect = scaleRect(85, 85, 50, 14, scale);
	(new GUI::ButtonWidget(this, rect.left, rect.top, rect.width(), rect.height(), cancelString))->setCmd(GUI::kCloseCmd);

	rect = scaleRect(7, 23, 144, 14, scale);
	_nickText = new GUI::EditTextWidget(this, rect.left, rect.top, rect.width(), rect.height(), nick);

	rect = scaleRect(7, 62, 144, 14, scale);
	_nameText = new GUI::EditTextWidget(this, rect.left, rect.top, rect.width(), rect.height(), name);

	reflowLayout();
	setResult(0);
}

Common::String HatchSequenceDialog::nick() const {
	return _nickText->getEditString();
}

Common::String HatchSequenceDialog::name() const {
	return _nameText->getEditString();
}

void HatchSequenceDialog::reflowLayout() {
	this->_x = (g_system->getOverlayWidth() - _w) / 2;
	this->_y = (g_system->getOverlayHeight() - _h) / 2;
}

void HatchSequenceDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kCloseCmd) {
		if (_tamaPlugin->dialogPrompt(TamaPlugin::kPromptTryAgain, 0, TamaPlugin::kDialogYesNoFlag) == TamaPlugin::kWinMessageNo)
			close();
		else
			setFocusWidget(_nickText);
		return;
	}
	else if (cmd != kCmdOK)
		return;

	const auto nick = Common::String(_nickText->getEditString());
	if (nick.empty()) {
		_tamaPlugin->dialogPrompt(TamaPlugin::kPromptMissingNick, 0, 0);
		setFocusWidget(_nickText);
		return;
	}

	// TODO: Check if nick is already in scrapbook
	if (nick.size() > 63) {
		_tamaPlugin->dialogPrompt(TamaPlugin::KPromptInvalidNick, 0, 0);
		setFocusWidget(_nickText);
		_nickText->setCaretPos(0);
		_nickText->setSelectionOffset(nick.size());
		return;
	}

	if (_tamaPlugin->dialogPrompt(TamaPlugin::kPromptConfirmNick, nick.c_str(), TamaPlugin::kDialogYesNoFlag) == TamaPlugin::kWinMessageNo) {
		setFocusWidget(_nickText);
		return;
	}

	setResult(1);
	close();
}

}

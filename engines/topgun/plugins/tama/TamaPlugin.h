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

#ifndef TOPGUN_TAMAPLUGIN_H
#define TOPGUN_TAMAPLUGIN_H

#include "common/formats/winexe.h"
#include "topgun/plugins/IPlugin.h"

namespace TopGun {
class Tamago;

class TamaPlugin : public IPlugin {
	friend class HatchSequenceDialog;
public:
	TamaPlugin(TopGunEngine *engine);
	virtual ~TamaPlugin();

	virtual ScriptPluginProcedure *getScriptProcedure(const Common::String &name) override;

private:
	enum {
		kWinMessageOK = 1,
		kWinMessageYes = 6,
		kWinMessageNo = 7,
		kDialogYesNoFlag = 4,
		kDialogStripString = 7,

		// additional prompts used by HatchSequenceDialog
		KPromptInvalidNick = 7,
		kPromptConfirmNick = 13,
		kPromptMissingNick,
		kPromptTryAgain,
	};

	int32 tama7thMakePersistent(const int *args, uint32 argCount);
	int32 volumeGetIncrements(const int *args, uint32 argCount);
	int32 dialogPrompt(const int *args, uint32 argCount);
	int32 dialogPrompt(int32 promptId, const char *argString, int32 flags);
	int32 dialogHatchSequence(const int *args, uint32 argCount);
	int32 stubReturnZero(const int *args, uint32 argCount);
	int32 stubReturnOne(const int *args, uint32 argCount);
	int32 internetOpenURL(const int *args, uint32 argCount);
	int32 windowGenerateMouseMove(const int *args, uint32 argCount);
	int32 windowClose(const int *args, uint32 argCount);
	int32 windowSetName(const int *args, uint32 argCount);

	int32 editCtrlCreate(const int *args, uint32 argCount);
	int32 editCtrlGetText(const int *args, uint32 argCount);
	int32 editCtrlSetText(const int *args, uint32 argCount);

	int32 tamagoGetNumActive(const int *args, uint32 argCount);
	int32 tamagoNew(const int *args, uint32 argCount);
	//int32 tamagoOpen(const int *args, uint32 argCount);
	int32 tamagoClose(const int *args, uint32 argCount);
	int32 tamagoSave(const int *args, uint32 argCount);
	int32 tamagoAction(const int *args, uint32 argCount);
	int32 tamagoQuery(const int *args, uint32 argCount);

	static void removeWinAPIHotkey(Common::String &text);

	Common::ScopedPtr<Common::WinResources> _tamaResources;
	Common::String _editCtrlText;
	Common::Array<Tamago *> _tamagos;
};

}

#endif // TOPGUN_TAMAPLUGIN_H

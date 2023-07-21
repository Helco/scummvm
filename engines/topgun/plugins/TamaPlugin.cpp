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

#include "common/translation.h"
#include "gui/message.h"
#include "topgun/plugins/TamaPlugin.h"
#include "topgun/topgun.h"

namespace TopGun {

TamaPlugin::TamaPlugin(TopGunEngine *engine) : IPlugin(engine) {
	_tamaResources.reset(Common::WinResources::createFromEXE("TAMA7TH.R32"));
}

ScriptPluginProcedure *TamaPlugin::getScriptProcedure(const Common::String &name) {
	if (!name.compareToIgnoreCase("Tama7th_MakePersistent"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tama7thMakePersistent);
	else if (!name.compareToIgnoreCase("Volume_GetMidiIncrements"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::volumeGetIncrements);
	else if (!name.compareToIgnoreCase("Volume_GetWaveIncrements"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::volumeGetIncrements);
	else if (!name.compareToIgnoreCase("Dialog_SignalAttention"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Dialog_Prompt"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogPrompt);
	else if (!name.compareToIgnoreCase("Save_MakeFileName"))
		// this would create an absolute path from a relative one, no need for ScummVM
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("TamagoGetNumActive"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoGetNumActive);
	else if (!name.compareToIgnoreCase("Window_GenerateMouseMove"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::windowGenerateMouseMove);
	// TODO: Implement those stubs
	else if (!name.compareToIgnoreCase("Volume_GetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_GetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_SetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Volume_SetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Window_ShowFullScreen"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Window_Restore"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Window_Show"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("TamagoGetNumScrap"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogSignalAttention);
	else if (!name.compareToIgnoreCase("Dialog_SetLanguage"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("Internet_OpenURL"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::internetOpenURL);
	else
		return nullptr;
}

int32 TamaPlugin::tama7thMakePersistent(const int32 *args, uint32 argCount) {
	return argCount < 1 ? 0 : args[0];
}

int32 TamaPlugin::volumeGetIncrements(const int32 *args, uint32 argCount) {
	return 32;
}

int32 TamaPlugin::dialogSignalAttention(const int32 *args, uint32 argCount) {
	// if we really wanted to we could check if the game is in focus and if not beep
	return 0;
}

struct DialogPromptData
{
	uint32 textResource;
	uint32 defaultButtonResource;
	uint32 altButtonResource;
	bool formatText;
};

constexpr int32 kWinMessageOK = 1;
constexpr int32 kWinMessageYes = 6;
constexpr int32 kWinMessageNo = 7;
constexpr int32 kDialogYesNoFlag = 4;
constexpr int32 kDialogStripString = 7;
constexpr DialogPromptData kDialogPrompts[] = {
	{ 8195, 8196, 8197, false }, // hatch now or later?
	{ 0, 0, 0, false }, // undefined prompt id
	{ 8213, 0, 0, false }, // press egg to hatch later
	{ 8214, 0, 0, true }, // error unable to open tamagotchi
	{ 8215, 0, 0, true }, // error unable to create tamagotchi
	{ 0, 0, 0, false },
	{ 8235, 0, 0, false }, // sure to send tamagotchi home early?
	{ 8217, 0, 0, false }, // enter valid nickname
	{ 8236, 0, 0, true }, // nickname has been changed to
	{ 0, 0, 0, false },
	{ 8243, 0, 0, false }, // you have not changed nickname
	{ 8246, 0, 0, false }, // sure to take tamagotchi out of care center
	{ 8248, 0, 0, false } // click on egg to begin hatching
};

static void removeWinAPIHotkey(Common::String &text) {
	auto index = text.findFirstOf('&');
	if (index != Common::String::npos)
		text.deleteChar(index);
}

int32 TamaPlugin::dialogPrompt(const int32 *args, uint32 argCount) {
	// TODO: icon flags and the title text are currently ignored
	if (argCount != 3)
		error("Expected three arguments for Dialog_Prompt");
	if (args[0] < 0 || args[0] >= sizeof(kDialogPrompts) / sizeof(DialogPromptData) ||
		kDialogPrompts[args[0]].textResource == 0)
		return 0;

	const auto &data = kDialogPrompts[args[0]];
	auto text = _tamaResources->loadString(data.textResource);
	if (data.formatText && args[1]) {
		auto arg = _engine->getScript()->getString(args[1]);
		text = Common::String::format(text.c_str(), arg.c_str());
	}
	if (args[0] == kDialogStripString) {
		auto arg = _engine->getScript()->getString(args[1]);
		auto i = arg.findFirstNotOf(' ');
		if (i == Common::String::npos)
			arg.clear();
		else if (i > 0)
			arg.erase(0, i);
		_engine->getScript()->setString(args[1], arg);
	}

	Common::String defaultButton, altButton;
	if (data.defaultButtonResource) {
		defaultButton = _tamaResources->loadString(data.defaultButtonResource);
		altButton = _tamaResources->loadString(data.altButtonResource);
	}
	else if (args[2] & kDialogYesNoFlag) {
		defaultButton = _("Yes");
		altButton = _("No");
	}
	else
		defaultButton = _("OK");

	removeWinAPIHotkey(defaultButton);
	removeWinAPIHotkey(altButton);
	
	GUI::MessageDialog dialog(text, defaultButton, altButton);
	auto positive = dialog.runModal() == GUI::kMessageOK;
	if (altButton.empty())
		return kWinMessageOK;
	else if (data.altButtonResource != 0) // custom dialogs return a sensible true/false
		return positive;
	else
		return positive ? kWinMessageYes : kWinMessageNo;
}

int32 TamaPlugin::stubReturnOne(const int32 *args, uint32 argCount) {
	return 1;
}

int32 TamaPlugin::internetOpenURL(const int32 *args, uint32 argCount) {
	if (argCount < 1)
		error("Invalid number of arguments for Internet_OpenURL");
	auto url = _engine->getScript()->getString(args[0]);
	warning("stub: Internet_OpenURL for %s", url.c_str());
	return 0;
}

constexpr const char *kSectionActive = "active";

int32 TamaPlugin::tamagoGetNumActive(const int32 *args, uint32 argCount) {
	auto &ini = _engine->getSavestate()->getINIFile();
	if (!ini.hasSection(kSectionActive))
		return 0;
	const auto section = ini.getKeys(kSectionActive);
	return section.size();
}

int32 TamaPlugin::windowGenerateMouseMove(const int32 *args, uint32 argCount) {
	Common::Event event;
	event.type = Common::EVENT_MOUSEMOVE;
	// TODO: We should probably set the current mouse pos in this event
	event.relMouse = Common::Point(0, 0);
	_engine->getEventManager()->pushEvent(event);
	return 1;
}

}

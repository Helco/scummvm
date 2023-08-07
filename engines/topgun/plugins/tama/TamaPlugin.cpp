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
#include "topgun/topgun.h"
#include "topgun/plugins/tama/TamaPlugin.h"
#include "topgun/plugins/tama/Tamago.h"
#include "topgun/plugins/tama/HatchSequenceDialog.h"

namespace TopGun {

TamaPlugin::TamaPlugin(TopGunEngine *engine) : IPlugin(engine) {
	_tamaResources.reset(Common::WinResources::createFromEXE("TAMA7TH.R32"));
}

TamaPlugin::~TamaPlugin() {
	for (auto tamago : _tamagos) {
		if (tamago != nullptr)
			delete tamago;
	}
}

ScriptPluginProcedure *TamaPlugin::getScriptProcedure(const Common::String &name) {
	if (!name.compareToIgnoreCase("Tama7th_MakePersistent"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tama7thMakePersistent);
	else if (!name.compareToIgnoreCase("Volume_GetMidiIncrements"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::volumeGetIncrements);
	else if (!name.compareToIgnoreCase("Volume_GetWaveIncrements"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::volumeGetIncrements);
	else if (!name.compareToIgnoreCase("Dialog_SignalAttention"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Dialog_Prompt"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogPrompt);
	else if (!name.compareToIgnoreCase("Dialog_HatchSequence"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::dialogHatchSequence);
	else if (!name.compareToIgnoreCase("Save_MakeFileName"))
		// this would create an absolute path from a relative one, no need for ScummVM
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("Window_GenerateMouseMove"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::windowGenerateMouseMove);
	else if (!name.compareToIgnoreCase("Window_Close"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::windowClose);

	else if (!name.compareToIgnoreCase("EditCtrl_Create"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::editCtrlCreate);
	else if (!name.compareToIgnoreCase("EditCtrl_Destroy"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("EditCtrl_GetText"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::editCtrlGetText);
	else if (!name.compareToIgnoreCase("EditCtrl_HasFocus"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("EditCtrl_KillFocus"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("EditCtrl_SetFocus"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("EditCtrl_SetText"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::editCtrlSetText);
	else if (!name.compareToIgnoreCase("EditCtrl_Show"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("EditCtrl_UpdateWindow"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);

	else if (!name.compareToIgnoreCase("TamagoGetNumActive"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoGetNumActive);
	else if (!name.compareToIgnoreCase("TamagoNew"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoNew);
	//else if (!name.compareToIgnoreCase("TamagoOpen"))
	//	return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoOpen);
	else if (!name.compareToIgnoreCase("TamagoClose"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoClose);
	else if (!name.compareToIgnoreCase("TamagoSave"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoSave);
	else if (!name.compareToIgnoreCase("TamagoAction"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoAction);
	else if (!name.compareToIgnoreCase("TamagoQuery"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::tamagoQuery);

	// TODO: Implement those stubs
	else if (!name.compareToIgnoreCase("Volume_GetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Volume_GetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Volume_SetMidiVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Volume_SetWaveVolume"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Window_ShowFullScreen"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Window_Restore"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Window_Show"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("TamagoIsScreenSaver"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("TamagoMakeScreenSaver"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("TamagoGetNumScrap"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnZero);
	else if (!name.compareToIgnoreCase("Dialog_SetLanguage"))
		return new ScriptPluginProcedureMem<TamaPlugin>(this, &TamaPlugin::stubReturnOne);
	else if (!name.compareToIgnoreCase("Help_Show"))
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

int32 TamaPlugin::stubReturnZero(const int32 *args, uint32 argCount) {
	return 0;
}

int32 TamaPlugin::stubReturnOne(const int32 *args, uint32 argCount) {
	return 1;
}

struct DialogPromptData
{
	uint32 textResource;
	uint32 defaultButtonResource;
	uint32 altButtonResource;
	bool formatText;
};

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
	{ 8248, 0, 0, false }, // click on egg to begin hatching
	// non-original prompts
	{ 8211, 0, 0, true }, // confirm nickname
	{ 8210, 0, 0, false }, // nickname is missing
	{ 8216, 0, 0, false }, // try again?
};

void TamaPlugin::removeWinAPIHotkey(Common::String &text) {
	auto index = text.findFirstOf('&');
	if (index != Common::String::npos)
		text.deleteChar(index);
}

int32 TamaPlugin::dialogPrompt(const int32 *args, uint32 argCount) {
	// TODO: icon flags and the title text are currently ignored
	if (argCount != 3)
		error("Expected three arguments for Dialog_Prompt");

	Common::String stringArg;
	if (args[1])
		stringArg = _engine->getScript()->getString(args[1]);
	if (args[0] == kDialogStripString) {
		auto i = stringArg.findFirstNotOf(' ');
		if (i == Common::String::npos)
			stringArg.clear();
		else if (i > 0)
			stringArg.erase(0, i);
		_engine->getScript()->setString(args[1], stringArg);
	}

	return dialogPrompt(args[0], args[1] ? stringArg.c_str() : nullptr, args[2]);
}

int32 TamaPlugin::dialogPrompt(int32 promptId, const char *stringArg, int32 flags) {
	if (promptId < 0 || promptId >= sizeof(kDialogPrompts) / sizeof(DialogPromptData) ||
		kDialogPrompts[promptId].textResource == 0)
		return 0;

	const auto &data = kDialogPrompts[promptId];
	auto text = _tamaResources->loadString(data.textResource);
	if (data.formatText && stringArg)
		text = Common::String::format(text.c_str(), stringArg);

	Common::String defaultButton, altButton;
	if (data.defaultButtonResource) {
		defaultButton = _tamaResources->loadString(data.defaultButtonResource);
		altButton = _tamaResources->loadString(data.altButtonResource);
	}
	else if (flags & kDialogYesNoFlag) {
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

int32 TamaPlugin::dialogHatchSequence(const int32 *args, uint32 argCount) {
	if (argCount < 2)
		error("Invalid number of arguments for Dialog_HatchSequence");
	auto nick = _engine->getScript()->getString(args[0]);
	auto name = _engine->getScript()->getString(args[1]);
	auto dialog = new HatchSequenceDialog(this, nick, name);
	int32 result = dialog->runModal();
	if (result) {
		_engine->getScript()->setString(args[0], dialog->nick());
		_engine->getScript()->setString(args[1], dialog->name());
	}
	return result;
}

int32 TamaPlugin::internetOpenURL(const int32 *args, uint32 argCount) {
	if (argCount < 1)
		error("Invalid number of arguments for Internet_OpenURL");
	auto url = _engine->getScript()->getString(args[0]);
	warning("stub: Internet_OpenURL for %s", url.c_str());
	return 0;
}

constexpr const char *kSectionActive = "active";

int32 TamaPlugin::windowGenerateMouseMove(const int32 *args, uint32 argCount) {
	Common::Event event;
	event.type = Common::EVENT_MOUSEMOVE;
	// TODO: We should probably set the current mouse pos in this event
	event.relMouse = Common::Point(0, 0);
	_engine->getEventManager()->pushEvent(event);
	return 1;
}

int32 TamaPlugin::windowClose(const int32 *args, uint32 argCount) {
	_engine->quitGame();
	return 1;
}

int32 TamaPlugin::editCtrlCreate(const int32 *args, uint32 argCount) {
	if (argCount != 6)
		error("Invalid number of arguments for EditCtrl_Create");
	_editCtrlText = _engine->getScript()->getString(args[5]);
	return 1337; // canary value, original would be handle to WinAPI edittext widget
	// maybe we could replace this with a dynamic sprite with text input. But that is not implemented either
}

int32 TamaPlugin::editCtrlSetText(const int32 *args, uint32 argCount) {
	if (argCount != 2)
		error("Invalid number of arguments for EditCtrl_SetText");
	_editCtrlText = _engine->getScript()->getString(args[1]);
	return 1;
}

int32 TamaPlugin::editCtrlGetText(const int32 *args, uint32 argCount) {
	if (argCount != 2)
		error("Invalid number of arguments for EditCtrl_GetText");
	_engine->getScript()->setString(args[1], _editCtrlText);
	return 1;
}

int32 TamaPlugin::tamagoGetNumActive(const int32 *args, uint32 argCount) {
	auto &ini = _engine->getSavestate()->getINIFile();
	if (!ini.hasSection(kSectionActive))
		return 0;
	const auto section = ini.getKeys(kSectionActive);
	return section.size();
}

int32 TamaPlugin::tamagoNew(const int *args, uint32 argCount) {
	if (argCount != 4)
		error("Invalid number of arguments for TamagoNew");
	// second argument is unused in the original game
	const auto nick = _engine->getScript()->getString(args[0]);
	auto tamago = new Tamago(_tamagos.size(), _engine);
	tamago->createNew(nick, args[2], args[3]);
	_tamagos.push_back(tamago);
	return tamago->id();
}

int32 TamaPlugin::tamagoClose(const int *args, uint32 argCount) {
	if (argCount != 1)
		error("Invalid number of arguments for TamagoClose");
	auto tamago = _tamagos[args[0]];
	if (!tamago->query(TamagoQuery::kGoneHome, 0))
		tamagoSave(args, argCount);
	delete tamago;
	_tamagos[args[0]] = nullptr;
	return 0;
}

int32 TamaPlugin::tamagoSave(const int *args, uint32 argCount) {
	if (argCount != 1)
		error("Invalid number of arguments for TamagoSave");
	warning("stub: Unimplemented procedure TamagoSave");
	return 1;
}

int32 TamaPlugin::tamagoAction(const int *args, uint32 argCount) {
	if (argCount != 3)
		error("Invalid number of arguments for TamagoAction");
	return _tamagos[args[0]]->action((TamagoAction)args[1], args[2]);
}

int32 TamaPlugin::tamagoQuery(const int *args, uint32 argCount) {
	if (argCount != 3)
		error("Invalid number of arguments for TamagoQuery");
	return _tamagos[args[0]]->query((TamagoQuery)args[1], args[2]);
}

}

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

#ifndef EDNA_SCRIPTLINE_H
#define EDNA_SCRIPTLINE_H

// Including script.h is necessary (instead of just util.h) to fix a MSVC bug
// regarding pointer-to-member-functions of incomplete types
#include "edna/script.h"

namespace Edna {

class Script;
struct ScriptCommand;

/**
 * @brief Runs a script line of a specific command
 * @returns true if next line should be executed immediately
 */
using ScriptHandler = bool (Script:: *)(const ScriptCommand &line);

struct ScriptCommand {
	ScriptCommand();
	ScriptCommand(char *line); ///< line is modified to insert null-terminators

	void debugPrintLog() const;
	void debugPrintConsole() const;

	ScriptHandler _handler = nullptr; ///< if nullptr the script line was not parsed
	const char *_function = nullptr;
	uint32 _fullLength = 0; ///< for debug print purposes as brackets/commas were replaced upon parsing
	Common::SharedPtr<const ScriptCommand> _thenLine = nullptr; ///< only for ifActive / ifItemActive
	// using an allocation for _thenLine saves around 2MiB of memory

	union {
		// TODO: AchievementKind _achievement;
		RoomObjectId _ifActive;
		ItemId _ifItemActive;
		struct {
			const char *_text;
			NPCId _npc;
		} _say; ///< also used for think
		struct {
			Common::Point _pos; ///< optional, kInvalidPoint otherwise
			const char *_sound; ///< optional
			const char *_text;
		} _saySound = {};
		ChoiceSetId _choiceSet;
		struct {
			ChoiceSetId _set;
			uint32 _line;
			bool _active;
		} _toggleChoice;
		struct {
			RoomObjectId _object;
			PlayerAction _action;
			ScriptId _newScript;
		} _changeInteraction;
		struct {
			ItemId _item;
			PlayerAction _action;
			ScriptId _newScript;
		} _changeItemInteraction;
		struct {
			ItemId _item;
			const char *_image;
		} _changeItemImage;
		struct {
			ItemId _item;
			RoomObjectId _object;
			ScriptId _newScript;
		} _changeRoomItemInteraction;
		struct {
			ItemId _item1, _item2;
			ScriptId _newScript;
		} _changeItemItemInteraction;
		struct {
			ChoiceSetId _set;
			uint32 _line;
			ScriptId _newScript;
		} _changeChoiceScript;
		ScriptId _script;
		struct {
			RoomObjectId _object;
			RoomId _target;
		} _exit;
		struct {
			RoomId _target;
			Common::Point _walkIn;
			Direction _direction;
		} _paramExit;
		struct {
			uint32 _duration;
			bool _isWhite; ///< otherwise black
			bool _isFadeIn;
		} _fade; ///< both fadein and fadeout
		bool _tempoMorphAlternateSound; ///< both tempomorphen and tempomorph2
		struct {
			Common::Point _target;
			NPCId _npc;
		} _walk; ///< used for all walk commands
		struct {
			ActionModeId _actionMode;
			uint32 _duration;
			NPCId _npc;
		} _animateCharacter;
		RoomObjectId _animateObject;
		Common::Point _putTarget;
		uint32 _waitDuration;
		ItemId _toggleItem; ///< both itemActivate(Sound) and itemDeactivate
		RoomObjectId _toggleObject; ///< both activate and inactivate
		struct {
			TimerId _timer;
			bool _active;
		} _toggleTimer;
		struct {
			Direction _direction;
			NPCId _npcId;
		} _lookAt; ///< both lookAt and npcLookAt
	} _args;
};

}

#endif // EDNA_SCRIPTLINE_H

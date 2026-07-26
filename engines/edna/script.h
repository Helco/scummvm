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

#ifndef EDNA_SCRIPT_H
#define EDNA_SCRIPT_H

#include "edna/util.h"

#include "audio/mixer.h"

namespace Edna {

class Game;
class Sprite;
struct ScriptCommand;
struct GameTransition;

class Script {
public:
	Script(Game &game, const GameTransition &transition);

	inline bool isScriptRunning() const { return _isScriptRunning; }
	inline ScriptId scriptId() const { return _scriptId; }
	inline uint32 scriptLine() const { return _scriptLine; }

	void resume();
	void runNew(ScriptId scriptId, uint32 firstLine = 1);
	void stop(); ///< currently only used for debugging
	bool isPerforming(); ///< this function has side-effects...

private:
	friend struct ScriptCommand;
	bool opAchievement(const ScriptCommand &line);
	bool opIfActive(const ScriptCommand &line);
	bool opIfItemActive(const ScriptCommand &line);
	bool opSay(const ScriptCommand &line);
	bool opThink(const ScriptCommand &line);
	bool opSayNpc(const ScriptCommand &line);
	bool opSaySound(const ScriptCommand &line); ///< also saySoundFile and (unused) saySoundP
	bool opChoice(const ScriptCommand &line);
	bool opToggleChoice(const ScriptCommand &line);
	bool opChangeInteraction(const ScriptCommand &line);
	bool opChangeItemInteraction(const ScriptCommand &line);
	bool opChangeItemImage(const ScriptCommand &line);
	bool opChangeRoomItemInteraction(const ScriptCommand &line);
	bool opChangeInvOBMScript(const ScriptCommand &line);
	bool opChangeChoiceScript(const ScriptCommand &line);
	bool opScript(const ScriptCommand &line);
	bool opExit(const ScriptCommand &line);
	bool opParamExit(const ScriptCommand &line);
	bool opFade(const ScriptCommand &line); ///< both fadeIn and fadeOut
	bool opTempoMorph(const ScriptCommand &line); ///< both tempomorphen and tempomorph2
	bool opAnimatePlayer(const ScriptCommand &line);
	bool opAnimatePlayerP(const ScriptCommand &line);
	bool opAnimateNpc(const ScriptCommand &line);
	// bool opAnimateNpcP(const ScriptCommand &line); (unused)
	bool opWalk(const ScriptCommand &line);
	// bool opWalkP(const ScriptCommand &line); (unused)
	bool opWalkNpc(const ScriptCommand &line);
	bool opWalkNpcP(const ScriptCommand &line);
	bool opFreeWalk(const ScriptCommand &line);
	bool opFreeWalkNpc(const ScriptCommand &line);
	// bool opPutNpc(const ScriptCommand &line); (unused)
	bool opPutPlayer(const ScriptCommand &line);
	bool opWait(const ScriptCommand &line);
	bool opItemActivate(const ScriptCommand &line);
	bool opItemActivateSound(const ScriptCommand &line);
	bool opItemDeactivate(const ScriptCommand &line);
	bool opActivate(const ScriptCommand &line);
	bool opDeactivate(const ScriptCommand &line);
	bool opToggleTimer(const ScriptCommand &line);
	bool opAnimateObject(const ScriptCommand &line);
	bool opLookAt(const ScriptCommand &line);
	bool opNpcLookAt(const ScriptCommand &line);
	// bool opEnd(const ScriptCommand &line); (unused)

	Common::String speechPath();
	void toggleObject(RoomObjectId objectId, bool isActive);

	Game &_game;
	ScriptId _scriptId = 0;
	uint32 _scriptLine = 0;
	bool _isScriptRunning = false,
		_isPerforming = false,
		_parallelPerformance = false;
	RoomObjectId _currentNpc = 0; ///< we use an ID because the object might get freed
	Audio::SoundHandle _currentSound = {};
	uint32 _currentSoundDuration = 0; ///< only used if no sound is played
	Sprite *_soundText = nullptr;
};

}

#endif // EDNA_SCRIPT_H

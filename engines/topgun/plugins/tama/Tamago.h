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

#ifndef TOPGUN_TAMAGO_H
#define TOPGUN_TAMAGO_H

#include "common/str.h"
#include "common/serializer.h"
#include "common/system.h"

namespace TopGun {
class TopGunEngine;
class Savestate;

enum class TamagoQuery : int32 {
	kHunger = 101,
	kHappyness = 102,
	kDiscipline = 103,
	kWeight = 104,
	kCanScold = 108,
	kCanEat = 110,
	kCanGiveMedicin = 112,
	kGoneHome = 113,
	kCanDaycare = 116,
	kCanBeWokenUp = 117,
	kGoneHomeReason = 118,
	kVType = 120,
	kAreLightsOff = 121,
	kFormatAge = 122,
	kIsValidExtraIntIndex = 123,
	kGetExtraInt = 124,
	kStatusFlags = 125,
	kYears = 126,
	kPoopCount = 127,
	kMistakesKind2 = 128,
	kMistakesKind1 = 129,
	kGeneration = 130,
	kWinsShellGame = 131,
	kStatusMode = 133
};

enum class TamagoAction : int32 {
	kGiveMeal = 200,
	kGiveSnack = 201,
	kScold = 202,
	kGiveMedicine = 203,
	kCleanPoop = 206,
	kWinGame = 207,
	kLoseGame = 208,
	kTimeSpeedUp = 209,
	kTimeSlowDown = 210,
	kTimeResetSpeed = 211,
	kSetAgeString = 213,
	kSetNextBigUpdateScript = 214,
	kToggleLights = 215,
	kHatch = 216,
	kSetEventScript = 217,
	kSendHome = 218,
	kSendToDaycare = 219,
	kWakeUp = 220,
	kSetType = 230
};

enum class TamagoEvent : int32 {
	kGoneHome = 309,
	kChangedSleep = 311,
	kClearedStatusFlag = 313,
	kChangedType = 314,
	kChangedStatus = 315,
	kUnhappyInDaycare = 316
};

enum class TamagoSender {
	kPropertyTimer = 1,
	kUser,
	kBasicTimer
};

enum class TamagoGoneHomeReason {
	kNone = 0,
	kSentByUser = 1,
	kNeglected = 2
};

enum class TamagoType {
	// names are for first generation
	kEgg
};

enum class TamagoVisualType {

};

class Tamago : public Common::Serializable {
public:
	Tamago(TopGunEngine *engine);

	static Tamago *createNew(TopGunEngine *engine,
		const Common::String &nick,
		int32 generationCount,
		int32 eventScriptId);

	void update();
	int32 query(TamagoQuery query, int32 value);
	int32 action(TamagoAction action, int32 value);

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

private:
	struct PropertyTimer : public Common::Serializable {
		int32
			_value = 0,
			_valueDecrease = 0,
			_timer = 0,
			_timerStart = 0;

		virtual void saveLoadWithSerializer(Common::Serializer &ser);
	};
	struct BasicTimer : public Common::Serializable {
		bool _enabled = false;
		int32 _timer = 0;
		int32 _timerStart = 0;

		virtual void saveLoadWithSerializer(Common::Serializer &ser);
	};
	struct ClockTime : public Common::Serializable {
		int32 _hour = 0,
			_minute = 0;

		virtual void saveLoadWithSerializer(Common::Serializer &ser);
	};
	struct TimeDateEx : public TimeDate, public Common::Serializable {
		TimeDateEx();
		TimeDateEx(TimeDate timeDate);

		void advanceBySeconds(int seconds);
		int compare(const TimeDateEx &other) const;
		int differenceInSeconds(const TimeDateEx &other) const;
		int differenceInMinutes(const TimeDateEx &other) const;

		virtual void saveLoadWithSerializer(Common::Serializer &ser);
	};
	typedef void (Tamago:: *TimerFunction)(TamagoSender sender);

	void send(TamagoEvent event, int32 value);
	void sendAllStatusMessages();

	void simulateSingleMinute();
	void simulate(BasicTimer &timer, TimerFunction function);
	void simulate(PropertyTimer &timer, TimerFunction function);
	void disable(BasicTimer &timer, bool wasMistake, bool wasMistakeKind1);
	void simulateSleepCycle();
	void handleMistake(TamagoSender sender);
	void handleEvolve(TamagoSender sender);
	void handleGoingHome(TamagoSender sender);
	void handleUnhappyInTheDark(TamagoSender sender);
	void handleHappyness(TamagoSender sender);
	void handlePreNoHappyness(TamagoSender sender);
	void handleHunger(TamagoSender sender);
	void handlePreNoHunger(TamagoSender sender);
	void handleActingUp(TamagoSender sender);
	void handleNeglect(TamagoSender sender);
	void handleTandrum(TamagoSender sender);
	void handleAutosave(TamagoSender sender);

	void actionGiveMeal();
	void actionGiveSnack();
	void actionScold();
	void actionGiveMedicine();
	void actionCleanPoop();
	void actionFinishGame(bool didWin);
	void actionToggleLights();
	void actionSendToDaycare();
	void actionWakeUp();

private:
	// runtime data
	TopGunEngine *_engine;
	bool
		_isHatched = false,
		_messagesEnabled = false;
	int32
		_timeFactor = 1,
		_eventScript = 0,
		_nextBigUpdateScript = 0,
		_nextBigUpdateMinutes = 0,
		_ageStringId = 0;
	BasicTimer _timerAutosave;

	// persistent data
	Common::String _nick;
	bool _isAwake = false,
		_areLighstOn = false,
		_isInDaycare = false,
		_canSleep = false,
		_hasSleepCycle = false;
	TamagoGoneHomeReason _goneHomeReason = TamagoGoneHomeReason::kNone;
	TamagoType _type = TamagoType::kEgg;
	int32
		_generation = 0,
		_poopCount = 0,
		_invDiscipline = 0,
		_actingUpDuration = 0,
		_actingUpTimer = 0,
		_maxTandrums = 0,
		_unhappyCount = 0,
		_maxUnhappyCount = 0,
		_givenMedicine = 0,
		_necessaryMedicine = 0,
		_weight = 0,
		_minWeight = 0,
		_shellGameChance = 0,
		_years = 0,
		_snacks = 0,
		_totalMistakes = 0,
		_mistakes1 = 0,
		_mistakes2 = 0;
	ClockTime
		_sleepCycleStart,
		_sleepCycleEnd;
	TimeDateEx
		_lastUpdate,
		_lastSimulation,
		_hatched;
	PropertyTimer
		_propHunger,
		_propHappyness,
		_propNeglect,
		_propDaycareChance;
	BasicTimer
		_timerNoHunger,
		_timerPreNoHunger,
		_timerNoHappyness,
		_timerPreNoHappyness,
		_timerGoingHome,
		_timerPoop,
		_timerTandrum,
		_timerSick,
		_timerGrowth,
		_timerMistake,
		_timerKickOutOfDaycare,
		_timerUnhappyInTheDark;
};

}

#endif

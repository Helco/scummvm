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

#include "topgun/plugins/tama/Tamago.h"
#include "topgun/topgun.h"

namespace TopGun {

struct TamagoTypeInfo {
	int32 _hungerTime,
		_happynessTime,
		_poopTime,
		_growthTime,
		_minTandrums,
		_maxTandrums,
		_invDiscipline,
		_neglectTime,
		_neglectValueDecrease,
		_maxUnhappyCount,
		_necessaryMedicine,
		_weight,
		_sleepCycleStartHour,
		_sleepCycleEndHour,
		_sleepCycleInMinutes;
	uint32 _shellGameChance;
} static const kTamagoTypes[] = {
	{
		7, // _hungerTime
		8, // _happynessTime
		0, // _poopTime
		0, // _growthTime
		0, // _minTandrums
		0, // _maxTandrums
		100, // _invDiscipline
		30, // _uncaredTime
		100, // _uncaredValueDecrease
		0, // _maxUnhappyCount
		2, // _necessaryMedicine
		5, // _weight
		0, // _sleepCycleStartHour
		0, // _sleepCycleEndHour
		0, // _sleepCycleInMinutes
		0 // _shellGameChance
	},
	{
		6, // _hungerTime
		8, // _happynessTime
		17, // _poopTime
		60, // _growthTime
		0, // _minTandrums
		0, // _maxTandrums
		100, // _invDiscipline
		40, // _uncaredTime
		100, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		2, // _necessaryMedicine
		5, // _weight
		0, // _sleepCycleStartHour
		0, // _sleepCycleEndHour
		60, // _sleepCycleInMinutes
		10 // _shellGameChance
	},
	{
		50, // _hungerTime
		85, // _happynessTime
		240, // _poopTime
		1380, // _growthTime
		6, // _minTandrums
		8, // _maxTandrums
		100, // _invDiscipline
		60, // _uncaredTime
		20, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		2, // _necessaryMedicine
		10, // _weight
		20, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		780, // _sleepCycleInMinutes
		15 // _shellGameChance
	},
	{
		70, // _hungerTime
		105, // _happynessTime
		200, // _poopTime
		2220, // _growthTime
		3, // _minTandrums
		6, // _maxTandrums
		50, // _invDiscipline
		60, // _uncaredTime
		7, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		2, // _necessaryMedicine
		20, // _weight
		21, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		7 // _shellGameChance
	},
	{
		70, // _hungerTime
		105, // _happynessTime
		200, // _poopTime
		2220, // _growthTime
		9, // _minTandrums
		12, // _maxTandrums
		100, // _invDiscipline
		60, // _uncaredTime
		7, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		2, // _necessaryMedicine
		20, // _weight
		21, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		7 // _shellGameChance
	},
	{
		43, // _hungerTime
		65, // _happynessTime
		200, // _poopTime
		1380, // _growthTime
		2, // _minTandrums
		4, // _maxTandrums
		50, // _invDiscipline
		60, // _uncaredTime
		12, // _uncaredValueDecrease
		2, // _maxUnhappyCount
		2, // _necessaryMedicine
		20, // _weight
		21, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		10 // _shellGameChance
	},
	{
		43, // _hungerTime
		65, // _happynessTime
		200, // _poopTime
		1380, // _growthTime
		6, // _minTandrums
		8, // _maxTandrums
		100, // _invDiscipline
		60, // _uncaredTime
		12, // _uncaredValueDecrease
		2, // _maxUnhappyCount
		2, // _necessaryMedicine
		20, // _weight
		21, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		10 // _shellGameChance
	},
	{
		90, // _hungerTime
		195, // _happynessTime
		200, // _poopTime
		4680, // _growthTime
		0, // _minTandrums
		0, // _maxTandrums
		0, // _invDiscipline
		60, // _uncaredTime
		2, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		1, // _necessaryMedicine
		30, // _weight
		21, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		1 // _shellGameChance
	},
	{
		90, // _hungerTime
		195, // _happynessTime
		200, // _poopTime
		3120, // _growthTime
		4, // _minTandrums
		8, // _maxTandrums
		50, // _invDiscipline
		60, // _uncaredTime
		3, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		1, // _necessaryMedicine
		30, // _weight
		23, // _sleepCycleStartHour
		11, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		8 // _shellGameChance
	},
	{
		60, // _hungerTime
		30, // _happynessTime
		200, // _poopTime
		2880, // _growthTime
		12, // _minTandrums
		16, // _maxTandrums
		50, // _invDiscipline
		60, // _uncaredTime
		4, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		1, // _necessaryMedicine
		30, // _weight
		22, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		660, // _sleepCycleInMinutes
		5 // _shellGameChance
	},
	{
		90, // _hungerTime
		105, // _happynessTime
		200, // _poopTime
		780, // _growthTime
		0, // _minTandrums
		0, // _maxTandrums
		0, // _invDiscipline
		60, // _uncaredTime
		10, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		2, // _necessaryMedicine
		20, // _weight
		22, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		660, // _sleepCycleInMinutes
		9 // _shellGameChance
	},
	{
		105, // _hungerTime
		90, // _happynessTime
		200, // _poopTime
		780, // _growthTime
		1, // _minTandrums
		2, // _maxTandrums
		50, // _invDiscipline
		60, // _uncaredTime
		18, // _uncaredValueDecrease
		2, // _maxUnhappyCount
		3, // _necessaryMedicine
		20, // _weight
		22, // _sleepCycleStartHour
		10, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		12 // _shellGameChance
	},
	{
		60, // _hungerTime
		90, // _happynessTime
		200, // _poopTime
		1560, // _growthTime
		6, // _minTandrums
		8, // _maxTandrums
		100, // _invDiscipline
		60, // _uncaredTime
		9, // _uncaredValueDecrease
		2, // _maxUnhappyCount
		2, // _necessaryMedicine
		20, // _weight
		22, // _sleepCycleStartHour
		10, // _sleepCycleEndHour
		720, // _sleepCycleInMinutes
		10 // _shellGameChance
	},
	{
		90, // _hungerTime
		195, // _happynessTime
		200, // _poopTime
		4680, // _growthTime
		0, // _minTandrums
		0, // _maxTandrums
		0, // _invDiscipline
		60, // _uncaredTime
		3, // _uncaredValueDecrease
		1, // _maxUnhappyCount
		1, // _necessaryMedicine
		30, // _weight
		22, // _sleepCycleStartHour
		9, // _sleepCycleEndHour
		660, // _sleepCycleInMinutes
		7 // _shellGameChance
	}
};

TamagoVisualType convertTamagoTypeToVisualType(TamagoType type) {
	static constexpr TamagoVisualType visualTypes[] = {
		TamagoVisualType::kEgg,
		TamagoVisualType::kBabytchi,
		TamagoVisualType::kMarutchi,
		TamagoVisualType::kTamatchi,
		TamagoVisualType::kTamatchi,
		TamagoVisualType::kKuchitamatchi,
		TamagoVisualType::kKuchitamatchi,
		TamagoVisualType::kMametchi,
		TamagoVisualType::kGinjirotchi,
		TamagoVisualType::kMaskutchi,
		TamagoVisualType::kKuchipatchi,
		TamagoVisualType::kNyorotchi,
		TamagoVisualType::kTarakotchi,
		TamagoVisualType::kBill
	};
	return visualTypes[(int32)type];
}

void Tamago::PropertyTimer::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncAsSint32LE(_value);
	ser.syncAsSint32LE(_valueDecrease);
	ser.syncAsSint32LE(_timer);
	ser.syncAsSint32LE(_timerStart);
}

void Tamago::BasicTimer::start() {
	_enabled = true;
	_timer = _timerStart;
}

void Tamago::BasicTimer::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncAsByte(_enabled);
	ser.syncAsSint32LE(_timer);
	ser.syncAsSint32LE(_timerStart);
}

void Tamago::ClockTime::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncAsSint32LE(_hour);
	ser.syncAsSint32LE(_minute);
}

// this time handling is not very correct by global standards.
// however it is original to Tamagotchi: CD-ROM

constexpr int kSecondsPerMinute = 60;
constexpr int kMinutesPerHour = 60;
constexpr int kHoursPerDay = 24;
constexpr int kDaysPerWeek = 7;
constexpr int kMonthsPerYear = 12;
constexpr int kSecondsPerHour = kSecondsPerMinute * kMinutesPerHour;
constexpr int kSecondsPerDay = kSecondsPerHour * kHoursPerDay;
constexpr int kDaysPerMonth[] = {
	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

static int getDaysInMonth(int year, int mon) {
	if (mon != 2)
		return kDaysPerMonth[mon];
	// not correct by the Gregorian calendar, but 7th Level used this
	year += 1900;
	bool isLeapYear = year % 4 == 0 && (year % 100 != 0 || year % 200 == 0);
	return kDaysPerMonth[mon] + (int)isLeapYear;
}

Tamago::TimeDateEx::TimeDateEx() {
	tm_year = 0;
	tm_mon = 0;
	tm_mday = 0;
	tm_wday = 0;
	tm_hour = 0;
	tm_min = 0;
	tm_sec = 0;
}

Tamago::TimeDateEx::TimeDateEx(TimeDate timeDate) {
	tm_year = timeDate.tm_year;
	tm_mon = timeDate.tm_mon;
	tm_mday = timeDate.tm_mday;
	tm_wday = timeDate.tm_wday;
	tm_hour = timeDate.tm_hour;
	tm_min = timeDate.tm_min;
	tm_sec = timeDate.tm_sec;
}

static void wrappingAdd(int &remainder, int &quotient, const int add, const int dividend) {
	remainder += add;
	quotient += remainder / dividend;
	remainder %= dividend;
}

void Tamago::TimeDateEx::advanceBySeconds(int seconds) {
	int minutes = 0, hours = 0, days = 0;
	wrappingAdd(tm_sec, minutes, seconds, kSecondsPerMinute);
	wrappingAdd(tm_min, hours, minutes, kMinutesPerHour);
	wrappingAdd(tm_hour, days, hours, kHoursPerDay);
	tm_wday = (tm_wday + days) % kDaysPerWeek;

	auto daysInCurMonth = getDaysInMonth(tm_year, tm_mon);
	while (tm_mday + days > daysInCurMonth) {
		wrappingAdd(tm_mon, tm_year, 1, kMonthsPerYear);
		days -= daysInCurMonth - tm_mday + 1;
		tm_mday = 1;
		daysInCurMonth = getDaysInMonth(tm_year, tm_mon);
	}
	tm_mday += days;
}

int Tamago::TimeDateEx::compare(const TimeDateEx &other) const {
	if (tm_year != other.tm_year) return other.tm_year - tm_year;
	if (tm_mon != other.tm_mon) return other.tm_mon - tm_mon;
	if (tm_mday != other.tm_mday) return other.tm_mday - tm_mday;
	if (tm_hour != other.tm_hour) return other.tm_hour - tm_hour;
	if (tm_min != other.tm_min) return other.tm_min - tm_min;
	return other.tm_sec - tm_sec;
}

// these difference functions will only work if this >= other

int Tamago::TimeDateEx::differenceInMinutes(const TimeDateEx &other) const {
	int comparison = compare(other);
	if (comparison <= 0)
		return comparison;

	auto diffInDays = tm_mday - other.tm_mday;
	auto otherMon = other.tm_mon;
	auto otherYear = other.tm_year;
	while (tm_year != otherYear || tm_mon != otherMon) {
		diffInDays += getDaysInMonth(otherYear, otherMon);
		wrappingAdd(otherMon, otherYear, 1, kMonthsPerYear);
	}

	const auto diffInHours = diffInDays * kHoursPerDay + tm_hour - other.tm_hour;
	return diffInHours * kMinutesPerHour + tm_min - other.tm_min;
}

int Tamago::TimeDateEx::differenceInSeconds(const TimeDateEx &other) const {
	auto diffMinutes = differenceInMinutes(other);
	return diffMinutes * kSecondsPerMinute + tm_sec - other.tm_sec;
}

void Tamago::TimeDateEx::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncAsSint32LE(tm_year);
	ser.syncAsSint32LE(tm_mon);
	ser.syncAsSint32LE(tm_mday);
	ser.syncAsSint32LE(tm_wday);
	ser.syncAsSint32LE(tm_hour);
	ser.syncAsSint32LE(tm_min);
	ser.syncAsSint32LE(tm_sec);
}

template<typename T> static void syncEnumAsSint32LE(Common::Serializer &ser, T &value) {
	int32 intValue = (int32)value;
	ser.syncAsSint32LE(intValue);
	value = (T)intValue;
}

void Tamago::saveLoadWithSerializer(Common::Serializer &ser) {
	ser.syncString(_nick);
	ser.syncAsByte(_isAwake);
	ser.syncAsByte(_areLightsOn);
	ser.syncAsByte(_isInDaycare);
	ser.syncAsByte(_canSleep);
	ser.syncAsByte(_hasSleepCycle);
	syncEnumAsSint32LE(ser, _goneHomeReason);
	syncEnumAsSint32LE(ser, _type);
	ser.syncAsSint32LE(_generation);
	ser.syncAsSint32LE(_poopCount);
	ser.syncAsSint32LE(_invDiscipline);
	ser.syncAsSint32LE(_actingUpDuration);
	ser.syncAsSint32LE(_actingUpTimer);
	ser.syncAsSint32LE(_maxTandrums);
	ser.syncAsSint32LE(_unhappyCount);
	ser.syncAsSint32LE(_maxUnhappyCount);
	ser.syncAsSint32LE(_givenMedicine);
	ser.syncAsSint32LE(_necessaryMedicine);
	ser.syncAsSint32LE(_weight);
	ser.syncAsSint32LE(_minWeight);
	ser.syncAsSint32LE(_years);
	ser.syncAsSint32LE(_snacks);
	ser.syncAsSint32LE(_totalMistakes);
	ser.syncAsSint32LE(_mistakes1);
	ser.syncAsSint32LE(_mistakes2);
	ser.syncAsUint32LE(_shellGameChance);
	_sleepCycleStart.saveLoadWithSerializer(ser);
	_sleepCycleEnd.saveLoadWithSerializer(ser);
	_lastUpdate.saveLoadWithSerializer(ser);
	_lastSimulation.saveLoadWithSerializer(ser);
	_hatched.saveLoadWithSerializer(ser);
	_propHunger.saveLoadWithSerializer(ser);
	_propHappyness.saveLoadWithSerializer(ser);
	_propNeglect.saveLoadWithSerializer(ser);
	_propDaycareChance.saveLoadWithSerializer(ser);
	_timerNoHunger.saveLoadWithSerializer(ser);
	_timerPreNoHunger.saveLoadWithSerializer(ser);
	_timerNoHappyness.saveLoadWithSerializer(ser);
	_timerPreNoHappyness.saveLoadWithSerializer(ser);
	_timerGoingHome.saveLoadWithSerializer(ser);
	_timerPoop.saveLoadWithSerializer(ser);
	_timerTandrum.saveLoadWithSerializer(ser);
	_timerSick.saveLoadWithSerializer(ser);
	_timerGrowth.saveLoadWithSerializer(ser);
	_timerMistake.saveLoadWithSerializer(ser);
	_timerKickOutOfDaycare.saveLoadWithSerializer(ser);
	_timerUnhappyInTheDark.saveLoadWithSerializer(ser);
}

Tamago::Tamago(int32 id, TopGunEngine *engine) :
	_id(id),
	_engine(engine),
	_random("Uninitialized Tamago") {
}

void Tamago::createNew(const Common::String &nick, int32 generationCount, int32 eventScriptId) {
	_random = Common::RandomSource("Tamago " + nick);
	_nick = nick;
	_generation = _random.getRandomNumberRngSigned(0, generationCount - 1);
	_eventScript = eventScriptId;
	_timeFactor = 1;
	_isAwake = true;

	TimeDate curSysTime;
	g_system->getTimeAndDate(curSysTime);
	_hatched = _lastSimulation = _lastUpdate = curSysTime;

	_messagesEnabled = false;
	actionSetType(TamagoType::kEgg);
	_messagesEnabled = true;
	send(TamagoEvent::kCreated, 0);
	handleGrowth(TamagoSender::kUser);
}

void Tamago::update() {
	TimeDate curSysTime;
	g_system->getTimeAndDate(curSysTime);
	TimeDateEx curTime(curSysTime);

	auto diffInSeconds = curTime.differenceInSeconds(_lastUpdate) * _timeFactor;
	if (diffInSeconds < 0) {
		// somehow the last update was in the future, reset and try again later
		_lastUpdate = _lastSimulation = curTime;
		return;
	}
	if (diffInSeconds < 1)
		return;
	auto nextSimulation = _lastSimulation;
	nextSimulation.advanceBySeconds(diffInSeconds);
	int minutesToSimulate, nextBigUpdateMinutes = 0;
	if (_type == TamagoType::kEgg) {
		minutesToSimulate = diffInSeconds / kSecondsPerMinute;
		nextBigUpdateMinutes = _timerGrowth._timer;
	}
	else {
		minutesToSimulate = nextSimulation.differenceInMinutes(_lastSimulation);
		if (_isInDaycare)
			nextBigUpdateMinutes = _timerKickOutOfDaycare._timer;
	}

	if (minutesToSimulate > 0) {
		if (minutesToSimulate > 1) {
			_messagesEnabled = false;
			for (int i = 1; i < minutesToSimulate; i++) {
				simulateSingleMinute();
				_lastSimulation.advanceBySeconds(kSecondsPerMinute);
			}
			_messagesEnabled = true;
			sendAllStatusMessages();
		}
		_lastSimulation = nextSimulation;
		_lastUpdate = curTime;
		simulateSingleMinute();
	}

	if (_ageStringId)
		formatTimeDateToString(_lastSimulation, _ageStringId);

	if (_nextBigUpdateScript && _nextBigUpdateMinutes != nextBigUpdateMinutes) {
		_nextBigUpdateMinutes = nextBigUpdateMinutes;
		int32 args[] = { _id, 0, 0, nextBigUpdateMinutes };
		wrappingAdd(args[3], args[2], 0, kMinutesPerHour);
		wrappingAdd(args[2], args[1], 0, kHoursPerDay);
		_engine->getScript()->runMessage(_nextBigUpdateScript, 32, 4, args);
	}
}

int32 Tamago::query(TamagoQuery query, int32 value) {
	switch (query) {
	case TamagoQuery::kHunger: return _propHunger._value;
	case TamagoQuery::kHappyness: return _propHappyness._value;
	case TamagoQuery::kDiscipline: return (100 - _invDiscipline) / 5;
	case TamagoQuery::kWeight: return _weight;
	case TamagoQuery::kGoneHome: return hasGoneHome();
	case TamagoQuery::kGoneHomeReason: return (int32)_goneHomeReason;
	case TamagoQuery::kAreLightsOff: return !_areLightsOn;
	case TamagoQuery::kYears: return _years;
	case TamagoQuery::kPoopCount: return _poopCount;
	case TamagoQuery::kMistakesKind1: return _mistakes1;
	case TamagoQuery::kMistakesKind2: return _mistakes2;
	case TamagoQuery::kGeneration: return _generation;
	case TamagoQuery::kWinsShellGame: return _random.getRandomNumber(99) <= _shellGameChance;
	case TamagoQuery::kVisualType: return (int32)convertTamagoTypeToVisualType(_type);
	case TamagoQuery::kNick:
		_engine->getScript()->setString(value, _nick);
		break;
	case TamagoQuery::kFormatAge:
		formatTimeDateToString(_lastSimulation, value);
		break;
	case TamagoQuery::kHatchedAsString:
		return formatDateToString(_hatched, value);
	case TamagoQuery::kNonCriticalIssues:
		return !_timerSick._enabled && _poopCount < 4;
	case TamagoQuery::kCanOpenGamesMenu:
	case TamagoQuery::kCanScold:
	case TamagoQuery::kCanCleanPoop:
	case TamagoQuery::kCanEat:
	case TamagoQuery::kCanGiveMedicine:
		return
			_isAwake &&
			!_isInDaycare &&
			!hasGoneHome() &&
			_type != TamagoType::kEgg;
	case TamagoQuery::kCanOpenLightsMenu:
		return
			!_isInDaycare &&
			!hasGoneHome() &&
			_type != TamagoType::kEgg;
	case TamagoQuery::kCanBeSentHome:
	case TamagoQuery::kCanToggleLights:
	case TamagoQuery::kCanDaycare:
		return
			!hasGoneHome() &&
			_type != TamagoType::kEgg;
	case TamagoQuery::kCanPlayGames:
		return
			_isAwake &&
			!hasGoneHome() &&
			_type != TamagoType::kEgg &&
			!_timerTandrum._enabled &&
			!_timerSick._enabled;
	case TamagoQuery::kStatusFlags: {
		int32 flags = 0;
		if (!hasGoneHome() && !_isInDaycare && _isAwake) {
			if (_timerNoHappyness._enabled)
				flags |= (int32)TamagoStatusFlags::kSad;
			if (_timerNoHunger._enabled)
				flags |= (int32)TamagoStatusFlags::kHungry;
			if (_timerSick._enabled)
				flags |= (int32)TamagoStatusFlags::kSick;
			if (_timerTandrum._enabled)
				flags |= (int32)TamagoStatusFlags::kTandrum;
		}
		return flags;
	}
	case TamagoQuery::kStatusMode:
		if (hasGoneHome())
			return (int32)TamagoStatusMode::kGoneHome;
		else if (_type == TamagoType::kEgg)
			return (int32)TamagoStatusMode::kUnhatched;
		else if (_isInDaycare)
			return (int32)TamagoStatusMode::kInDaycare;
		else if (_isAwake)
			return (int32)TamagoStatusMode::kAwake;
		else
			return (int32)TamagoStatusMode::kAsleep;
	default:
		error("Unsupported Tamago query type: %d", (int)query);
	}
	return 0;
}

int32 Tamago::action(TamagoAction action, int32 value) {
	int32 result = 0;
	switch (action) {
	case TamagoAction::kGiveMeal: result = actionGiveMeal();
	case TamagoAction::kGiveSnack: result = actionGiveSnack();
	case TamagoAction::kScold: result = actionScold();
	case TamagoAction::kGiveMedicine: result = actionGiveMedicine();
	case TamagoAction::kCleanPoop: result = actionCleanPoop();
	case TamagoAction::kWinGame: result = actionFinishGame(true);
	case TamagoAction::kLoseGame: result = actionFinishGame(false);
	case TamagoAction::kTimeSpeedUp: _timeFactor += 5; break;
	case TamagoAction::kTimeSlowDown: _timeFactor = MAX(1, _timeFactor - 5); break;
	case TamagoAction::kTimeResetSpeed: _timeFactor = 1; break;
	case TamagoAction::kSetAgeString: _ageStringId = value; break;
	case TamagoAction::kSetNextBigUpdateScript:
		_nextBigUpdateMinutes = 0;
		_nextBigUpdateScript = value;
		break;
	case TamagoAction::kToggleLights: result = actionToggleLights();
	case TamagoAction::kHatch: _isHatched = value; break;
	case TamagoAction::kSetEventScript: _eventScript = value; break;
	case TamagoAction::kSendHome: result = actionSendHome(TamagoGoneHomeReason::kSentByUser);
	case TamagoAction::kSendToDaycare: result = actionSendToDaycare(); break;
	case TamagoAction::kWakeUp: result = actionWakeUp(); break;
	default:
		error("Unsupported Tamago action type: %d", (int)action);
	}

	handleAutosave(TamagoSender::kUser);
	return result;
}

void Tamago::send(TamagoEvent event, int32 value) {
	if (!_messagesEnabled || !_eventScript)
		return;

	if (_isInDaycare) {
		if (event == TamagoEvent::kSetStatusFlag)
			_timeFactor = 1;
		else if (event != TamagoEvent::kClearedStatusFlag)
			return;
	}

	const int32 args[] = { _id, (int32)event, value };
	_engine->getScript()->runMessage(_eventScript, 32, 3, args);
}

void Tamago::sendAllStatusMessages() {
	if (hasGoneHome()) {
		send(TamagoEvent::kAlreadyGoneHome, 0);
		send(TamagoEvent::kChangedType, (int32)convertTamagoTypeToVisualType(_type));
	} else if (_type == TamagoType::kEgg) {
		send(TamagoEvent::kStillEgg, 0);
	} else {
		send(_isAwake ? TamagoEvent::kStillAwake : TamagoEvent::kStillSleeping, _areLightsOn);
		send(TamagoEvent::kChangedType, (int32)convertTamagoTypeToVisualType(_type));
		send(TamagoEvent::kChangedSleep, _areLightsOn);
		send(TamagoEvent::kChangedStatus, 0);
	}
}

void Tamago::sendUnhappyDuringDaycare() {
	if (!_isInDaycare)
		return;
	send(TamagoEvent::kUnhappyInDaycare, !_timerSick._enabled && _poopCount < 3);
}

void Tamago::simulateSingleMinute() {
	if (hasGoneHome() || !_isHatched)
		return;
	if (_isInDaycare) {
		simulate(_timerKickOutOfDaycare, &Tamago::handleKickedOutOfDaycare);
		simulate(_propDaycareChance, nullptr);
	}
	if (_type == TamagoType::kEgg) {
		simulate(_timerGrowth, &Tamago::handleGrowth);
		simulate(_timerAutosave, &Tamago::handleAutosave);
		return;
	}

	simulate(_timerUnhappyInTheDark, &Tamago::handleUnhappyInTheDark);
	simulateSleepCycle();

	simulateComplexProperty(_propHunger, _timerNoHunger, _timerPreNoHunger, &Tamago::handleHunger, &Tamago::handlePreNoHunger);
	if (_isAwake)
		simulate(_timerGoingHome, &Tamago::handleGoingHome);
	simulateComplexProperty(_propHappyness, _timerNoHappyness, _timerPreNoHappyness, &Tamago::handleHappyness, &Tamago::handlePreNoHappyness);
	if (_isAwake)
		simulate(_timerPoop, &Tamago::handlePoop);
	if (_isAwake)
		simulate(_timerSick, &Tamago::handleNeglect);
	if (_isAwake && _unhappyCount < _maxUnhappyCount)
		simulate(_propNeglect, &Tamago::handleNeglect);
	simulate(_timerGrowth, &Tamago::handleGrowth);
	if (_isAwake)
		simulate(_timerMistake, &Tamago::handleMistake);
	if (_messagesEnabled)
		simulate(_timerAutosave, &Tamago::handleAutosave);
}

void Tamago::simulate(Tamago::BasicTimer &timer, TimerFunction function) {
	if (timer._enabled && --timer._timer <= 0) {
		timer._enabled = false;
		if (function != nullptr)
			(this->*function)(TamagoSender::kBasicTimer);
	}
}

void Tamago::simulate(Tamago::PropertyTimer &prop, TimerFunction function) {
	if (--prop._timer > 0)
		return;
	prop._value = MAX(0, prop._value - prop._valueDecrease);
	if (prop._value == 0 && function != nullptr)
		(this->*function)(TamagoSender::kPropertyTimer);
	prop._timer = prop._timerStart;
}

void Tamago::simulateComplexProperty(
	PropertyTimer &property,
	BasicTimer &critical,
	BasicTimer &leeway,
	TimerFunction propertyFunction,
	TimerFunction leewayFunction) {
	// Hunger and happyness are properties that go critical if depleted
	// however if the tamagotchi is still a baby there is additional leeway before the critical phase
	// additionally the property is not to be updated further if depleted

	if (_type == TamagoType::kBabytchi) {
		if (_isAwake)
			simulate(leeway, leewayFunction);
		if (_isAwake && !leeway._enabled)
			simulate(critical, propertyFunction);
		if (_isAwake && (!critical._enabled || leeway._enabled))
			simulate(property, propertyFunction);
	}
	else {
		if (_isAwake)
			simulate(critical, propertyFunction);
		if (_isAwake && !critical._enabled)
			simulate(property, propertyFunction);
	}
}

void Tamago::disable(Tamago::BasicTimer &timer, bool wasMistake, bool wasMistakeKind1) {
	timer._enabled = false;
	if (wasMistake) {
		_totalMistakes++;
		if (wasMistakeKind1)
			_mistakes1++;
		else
			_mistakes2++;
	}
}

void Tamago::simulateSleepCycle() {
	if (!_canSleep)
		return;

	const auto &typeInfo = kTamagoTypes[(int32)_type];
	const auto isSleepTime = this->isSleepTime();
	if (!_isAwake && !isSleepTime) {
		if (_type != TamagoType::kBabytchi) {
			_sleepCycleStart._minute = 0;
			_sleepCycleStart._hour = typeInfo._sleepCycleStartHour;
		}
		_isAwake = true;
		const auto wereLightsOn = _areLightsOn;
		_areLightsOn = false;
		_years++;
		if (!_isInDaycare)
			send(TamagoEvent::kWokeUp, 0);
		if (wereLightsOn)
			send(TamagoEvent::kChangedSleep, 1);
	}
	else if (_isAwake && isSleepTime) {
		_isAwake = false;
		if (_type != TamagoType::kBabytchi) {
			_sleepCycleEnd._minute = 0;
			_sleepCycleEnd._hour = typeInfo._sleepCycleEndHour;
		}
		if (!_isInDaycare)
			send(TamagoEvent::kFallenAsleep, 0);
		if (!_areLightsOn) {
			_timerUnhappyInTheDark.start();
			send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kUnhappyInTheDark);
		}
	}
}

TamagoSender Tamago::checkDaycare(TamagoSender sender) {
	if (_isInDaycare && sender == TamagoSender::kBasicTimer &&
		_random.getRandomNumberRngSigned(0, 99) <= _propDaycareChance._value)
		return TamagoSender::kUser;
	else
		return sender;
}

void Tamago::handleKickedOutOfDaycare(TamagoSender sender) {
	actionSendToDaycare();
}

void Tamago::handleMistake(TamagoSender sender) {
	_timerMistake.start();
	const auto &typeInfo = kTamagoTypes[(int32)_type];
	
	auto newHungerTime = MAX(2, _propHunger._timerStart - 20 * typeInfo._hungerTime / 100);
	if (_timerNoHunger._timerStart >= newHungerTime)
		_timerNoHunger._timerStart = newHungerTime - 1;
	_propHunger._timerStart = newHungerTime;

	auto newHappynessTime = MAX(2, _propHappyness._timerStart - 20 * typeInfo._happynessTime / 100);
	if (_timerNoHappyness._timerStart >= newHappynessTime)
		_timerNoHappyness._timerStart = newHappynessTime - 1;
	_propHappyness._timerStart = newHappynessTime;
}

void Tamago::handleGrowth(TamagoSender sender) {
	_timerGrowth._enabled = false;
	TamagoType oldType = _type,
		newType = TamagoType::kEgg;
	switch (oldType) {
	case TamagoType::kEgg:
		newType = TamagoType::kBabytchi;
		break;
	case TamagoType::kBabytchi:
		if (_mistakes1 > 2) {
			if (_mistakes1 > 3 || _mistakes2 > 1)
				newType = TamagoType::kKuchitamatchi6;
			else
				newType = TamagoType::kKuchitamatchi5;
		} else if (_mistakes2 > 1)
			newType = TamagoType::kTamatchi4;
		else
			newType = TamagoType::kTamatchi3;
		break;
	case TamagoType::kTamatchi3:
		if (_mistakes1 <= 1 && _mistakes2 < 2) {
			if (_mistakes2 == 0)
				newType = TamagoType::kMametchi;
			else
				newType = TamagoType::kGinjirotchi;
		} else if (_mistakes1 <= 2 && _mistakes2 >= 2)
			newType = TamagoType::kMaskutchi;
		else if (_mistakes1 < 3)
			newType = TamagoType::kTarakotchi;
		else if (_mistakes2 > 1) {
			if (_mistakes2 == 2 || _mistakes2 == 3)
				newType = TamagoType::kNyorotchi;
			else
				newType = TamagoType::kTarakotchi;
		} else
			newType = TamagoType::kKuchipatchi;
		break;
	case TamagoType::kTamatchi4:
		if (_mistakes1 > 3) {
			if (_mistakes2 > 7)
				newType = TamagoType::kTarakotchi;
			else
				newType = TamagoType::kNyorotchi;
		}
		else if (_mistakes2 > 1)
			newType = TamagoType::kMaskutchi;
		else
			newType = TamagoType::kGinjirotchi;
		break;
	case TamagoType::kKuchitamatchi5:
		if (_mistakes1 > 2)
			newType = TamagoType::kTarakotchi;
		else if (_mistakes2 > 1) {
			if (_mistakes2 == 2)
				newType = TamagoType::kNyorotchi;
			else
				newType = TamagoType::kTarakotchi;
		} else
			newType = TamagoType::kKuchipatchi;
		break;
	case TamagoType::kKuchitamatchi6:
		if (_mistakes2 > 5)
			newType = TamagoType::kTarakotchi;
		else
			newType = TamagoType::kNyorotchi;
		break;
	case TamagoType::kMaskutchi:
		if (_mistakes1 <= 5 && !_mistakes2)
			newType = TamagoType::kBill;
	}

	if (newType == TamagoType::kEgg) {
		const auto &typeInfo = kTamagoTypes[(int32)_type];
		_timerMistake._timerStart = 1440 - (typeInfo._sleepCycleInMinutes + 31 * (int32)oldType);
		_timerMistake.start();
	} else
		actionSetType(newType);
}

void Tamago::handleGoingHome(TamagoSender sender) {
	if (sender == TamagoSender::kBasicTimer)
		actionSendHome(TamagoGoneHomeReason::kNeglected);
	else
		disable(_timerGoingHome, false, false);
}

void Tamago::handleUnhappyInTheDark(TamagoSender sender) {
	sender = checkDaycare(sender);
	if (sender == TamagoSender::kBasicTimer) {
		disable(_timerUnhappyInTheDark, true, false);
		_propHappyness._value = MAX(0, _propHappyness._value - 1);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kUnhappyInTheDark);
	} else if (sender == TamagoSender::kUser) {
		disable(_timerUnhappyInTheDark, false, false);
		_areLightsOn = true;
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kUnhappyInTheDark);
		send(TamagoEvent::kChangedSleep, 0);
	}
}

void Tamago::handleHappyness(TamagoSender sender) {
	sender = checkDaycare(sender);
	switch (sender) {
	case TamagoSender::kPropertyTimer:
		_timerNoHappyness.start();
		send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kSad);
		break;
	case TamagoSender::kBasicTimer:
		disable(_timerNoHappyness, true, false);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kSad);
		if (_type == TamagoType::kBabytchi)
			_timerPreNoHappyness.start();
		else if (_timerMistake._enabled && _mistakes1 > 5)
			actionSendHome(TamagoGoneHomeReason::kNeglected);
		break;
	case TamagoSender::kUser:
		disable(_timerNoHappyness, false, false);
		disable(_timerPreNoHappyness, false, false);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kSad);
		if (_isInDaycare)
			_propHappyness._value = 4;
		break;
	}
}

void Tamago::handlePreNoHappyness(TamagoSender sender) {
	disable(_timerNoHappyness, false, false);
	_timerNoHappyness.start();
	send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kSad);
}

void Tamago::handleHunger(TamagoSender sender) {
	sender = checkDaycare(sender);
	switch (sender) {
	case TamagoSender::kPropertyTimer:
		if (!_timerGoingHome._enabled)
			_timerGoingHome.start();
		_timerNoHunger.start();
		send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kHungry);
		break;
	case TamagoSender::kBasicTimer:
		disable(_timerNoHunger, true, false);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kHungry);
		if (_type == TamagoType::kBabytchi)
			_timerPreNoHunger.start();
		else if (_timerMistake._enabled && _mistakes1 > 5)
			actionSendHome(TamagoGoneHomeReason::kNeglected);
		break;
	case TamagoSender::kUser:
		disable(_timerPreNoHunger, false, false);
		disable(_timerGoingHome, false, false);
		disable(_timerNoHunger, false, false);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kHungry);
		if (_isInDaycare)
			_propHunger._value = 4;
		break;
	}
}

void Tamago::handlePreNoHunger(TamagoSender sender) {
	disable(_timerPreNoHunger, false, false);
	_timerNoHunger.start();
	send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kHungry);
}

void Tamago::handlePoop(TamagoSender sender) {
	sender = checkDaycare(sender);
	if (sender == TamagoSender::kBasicTimer) {
		_poopCount++;
		int32 penalty = 0;
		if (_poopCount == 6)
			penalty = 30;
		else if (_poopCount == 7)
			penalty = 50;
		else if (_poopCount == 8)
			penalty = 100;
		_propNeglect._value = MAX(0, _propNeglect._value - penalty);
	}

	_timerPoop.start();
	send(TamagoEvent::kPooped, _poopCount);
	if (_poopCount >= 8 && !_timerSick._enabled && !_propNeglect._value) {
		_propNeglect._timer = 1;
		simulate(_propNeglect, &Tamago::handleNeglect);
	}
	sendUnhappyDuringDaycare();
}

void Tamago::handleNeglect(TamagoSender sender) {
	if (_isInDaycare && sender == TamagoSender::kBasicTimer)
		sender = TamagoSender::kUser;
	switch (sender) {
	case TamagoSender::kPropertyTimer:
		_givenMedicine = 0;
		_unhappyCount++;
		if (_unhappyCount < 3) {
			_timerSick.start();
			send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kSick);
			sendUnhappyDuringDaycare();
		} else
			actionSendHome(TamagoGoneHomeReason::kNeglected);
		break;
	case TamagoSender::kBasicTimer:
		disable(_timerSick, true, false);
		actionSendHome(TamagoGoneHomeReason::kNeglected);
		break;
	case TamagoSender::kUser:
		disable(_timerSick, false, false);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kSick);
		sendUnhappyDuringDaycare();
		break;
	}
}

void Tamago::handleTandrum(TamagoSender sender) {
	sender = checkDaycare(sender);
	switch (sender) {
	case TamagoSender::kPropertyTimer:
		_timerTandrum.start();
		send(TamagoEvent::kSetStatusFlag, (int32)TamagoStatusFlags::kTandrum);
		break;
	case TamagoSender::kBasicTimer:
		disable(_timerTandrum, true, true);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kTandrum);
		break;
	case TamagoSender::kUser:
		disable(_timerTandrum, false, false);
		send(TamagoEvent::kClearedStatusFlag, (int32)TamagoStatusFlags::kTandrum);
		break;
	}
}

void Tamago::handleAutosave(TamagoSender sender) {
	_timerAutosave.start();
	if (!_messagesEnabled)
		return;
	// TODO: Implement save
}

int32 Tamago::actionGiveMeal() {
	if (!query(TamagoQuery::kCanEat, 0) ||
		_propHunger._value >= 4 ||
		_timerTandrum._enabled ||
		_timerSick._enabled)
		return 0;
	_propHunger._value++;

	if (_type != TamagoType::kBabytchi)
		_weight = MIN(99, _weight + 1);
	disable(_timerPreNoHunger, false, false);
	disable(_timerGoingHome, false, false);

	if (_timerNoHunger._enabled)
		handleHunger(TamagoSender::kUser);
	else
		send(TamagoEvent::kChangedStatus, 0);
	return 1;
}

int32 Tamago::actionGiveSnack() {
	if (!query(TamagoQuery::kCanEat, 0))
		return 0;

	if (_type != TamagoType::kBabytchi)
		_weight = MIN(99, _weight + 2);

	_snacks++;
	if ((_snacks % 3) == 0)
		_propNeglect._value -= 10;
	_propHappyness._value = MIN(5, _propHappyness._value + 1);
	if (_timerNoHappyness._enabled)
		handleHappyness(TamagoSender::kUser);
	disable(_timerPreNoHappyness, false, false);
	send(TamagoEvent::kChangedStatus, 0);
	return 1;
}

int32 Tamago::actionScold() {
	if (!query(TamagoQuery::kCanScold, 0) ||
		!_timerTandrum._enabled ||
		_timerSick._enabled)
		return 0;

	_invDiscipline = MAX(0, _invDiscipline - 25);
	handleTandrum(TamagoSender::kUser);
	return 1;
}

int32 Tamago::actionGiveMedicine() {
	if (!query(TamagoQuery::kCanGiveMedicine, 0) ||
		!_timerSick._enabled)
		return 0;
	_givenMedicine++;
	if (_givenMedicine >= _necessaryMedicine) {
		_propNeglect._value = 100;
		handleNeglect(TamagoSender::kUser);
	}
	return 1;
}

int32 Tamago::actionCleanPoop() {
	_poopCount = 0;
	send(TamagoEvent::kChangedStatus, 0);
	return 1;
}

int32 Tamago::actionFinishGame(bool didWin) {
	if (didWin) {
		_propHappyness._value = MIN(5, _propHappyness._value + 1);
		if (_timerNoHappyness._enabled)
			handleHappyness(TamagoSender::kUser);
	}
	if (_type != TamagoType::kBabytchi)
		_weight = MAX(_minWeight, _weight - 1);
	send(TamagoEvent::kChangedStatus, 0);
	return 1;
}

int32 Tamago::actionToggleLights() {
	_areLightsOn = !_areLightsOn;
	if (_areLightsOn && _timerUnhappyInTheDark._enabled)
		handleUnhappyInTheDark(TamagoSender::kUser);
	send(TamagoEvent::kChangedSleep, _areLightsOn);
	return 1;
}

int32 Tamago::actionSendHome(TamagoGoneHomeReason reason) {
	_goneHomeReason = TamagoGoneHomeReason::kNeglected;
	if (reason != TamagoGoneHomeReason::kNeglecting)
		_goneHomeReason = reason;

	_isAwake = true;
	_areLightsOn = false;
	_isInDaycare = 0;
	_poopCount = 0;
	send(TamagoEvent::kGoneHome, (int32)reason);
	send(TamagoEvent::kChangedStatus, 0);
	handleAutosave(TamagoSender::kUser);
	return 1;
}

int32 Tamago::actionSendToDaycare()
{
	if (_isInDaycare)
		_isInDaycare = false;
	else {
		_isInDaycare = true;
		_timerKickOutOfDaycare.start();
		_propDaycareChance._timerStart = 780;
		_propDaycareChance._timer = 780;
		_propDaycareChance._valueDecrease = 5;
		_propDaycareChance._value = 95;
	}
	sendAllStatusMessages();
	return 1;
}

int32 Tamago::actionWakeUp() {
	if (!canBeWokenUp())
		return false;

	auto cycleEnd = _lastSimulation;
	cycleEnd.tm_hour = _sleepCycleEnd._hour;
	cycleEnd.tm_min = _sleepCycleEnd._minute;
	if (cycleEnd.compare(_lastSimulation) <= 1)
		return 1;

	const auto &typeInfo = kTamagoTypes[(int32)_type];
	int32 minutes = 1440 - (typeInfo._sleepCycleInMinutes + 31 * (int32)_type);
	auto sleepEnd = _lastSimulation;
	sleepEnd.advanceBySeconds(minutes * kSecondsPerMinute);

	_isAwake = true;
	_areLightsOn = false;
	_years++;
	_sleepCycleStart._hour = sleepEnd.tm_hour;
	_sleepCycleStart._minute = sleepEnd.tm_min;
	_sleepCycleEnd._hour = _lastSimulation.tm_hour;
	send(TamagoEvent::kWokeUp, 0);
	send(TamagoEvent::kChangedSleep, 1);
	return 1;
}

int32 Tamago::actionSetType(TamagoType newType) {
	_timerAutosave._timerStart = 5;
	_timerAutosave.start();
	_timerAutosave._timer = 0;

	const auto &typeInfo = kTamagoTypes[(int32)newType];
	if (newType == TamagoType::kBabytchi) {
		_timerPreNoHunger._timerStart = 1;
		_timerNoHunger._timerStart = 2;
		_timerPreNoHappyness._timerStart = 2;
		_timerNoHappyness._timerStart = 2;
		_timerUnhappyInTheDark._timerStart = 2;
		_propHappyness._timer = 2;
		_propHunger._timer = 0;
		_propNeglect._timer = 40;
		_timerNoHunger.start();
	} else {
		_timerNoHunger._timerStart = 15;
		_timerNoHappyness._timerStart = 15;
		_timerPreNoHunger._enabled = false;
		_timerPreNoHappyness._enabled = false;
		_timerUnhappyInTheDark._timerStart = 15;
	}

	auto prevType = _type;
	_type = newType;
	_unhappyCount = 0;
	_maxUnhappyCount = typeInfo._maxUnhappyCount;
	_mistakes1 = 0;
	_mistakes2 = 0;
	_timerSick._timerStart = 360;
	_timerKickOutOfDaycare._timerStart = 4320;
	_timerGoingHome._timerStart = 720;
	_propHunger._timerStart = typeInfo._hungerTime;
	_propHunger._valueDecrease = 1;
	_propHappyness._timerStart = typeInfo._happynessTime;
	_propHappyness._valueDecrease = 1;
	_necessaryMedicine = typeInfo._necessaryMedicine;
	_propNeglect._timerStart = typeInfo._neglectTime;
	_propNeglect._valueDecrease = typeInfo._neglectValueDecrease;
	_weight = typeInfo._weight;
	_minWeight = typeInfo._weight;
	_timerGrowth._timerStart = typeInfo._growthTime;
	_timerGrowth.start();
	_timerPoop._timerStart = typeInfo._poopTime;
	if (typeInfo._poopTime)
		_timerPoop.start();
	_maxTandrums = typeInfo._maxTandrums;
	_invDiscipline = typeInfo._invDiscipline;
	_timerTandrum._enabled = false;
	_shellGameChance = typeInfo._shellGameChance;

	if (typeInfo._maxTandrums && typeInfo._minTandrums) {
		auto tandrums = _random.getRandomNumberRngSigned(typeInfo._minTandrums, typeInfo._maxTandrums);
		_actingUpDuration = typeInfo._growthTime / tandrums;
		_actingUpTimer = typeInfo._growthTime / tandrums;
	}

	if (typeInfo._sleepCycleInMinutes) {
		_canSleep = true;
		_hasSleepCycle = true;
		if (typeInfo._sleepCycleStartHour && typeInfo._sleepCycleEndHour) {
			_sleepCycleStart._hour = typeInfo._sleepCycleStartHour;
			_sleepCycleEnd._hour = typeInfo._sleepCycleEndHour;
			_sleepCycleStart._minute = 0;
			_sleepCycleEnd._minute = 0;
		} else {
			auto minutes = _random.getRandomNumberRngSigned(5, typeInfo._growthTime - 5);
			auto startTime = _lastSimulation;
			startTime.advanceBySeconds(minutes * kSecondsPerMinute);
			auto endTime = startTime;
			endTime.advanceBySeconds(typeInfo._sleepCycleInMinutes * kSecondsPerMinute);
			_sleepCycleStart._hour = startTime.tm_hour;
			_sleepCycleStart._minute = startTime.tm_min;
			_sleepCycleEnd._hour = endTime.tm_hour;
			_sleepCycleEnd._minute = endTime.tm_min;
		}
	} else
		_canSleep = false;

	if (prevType == TamagoType::kEgg && newType != TamagoType::kEgg)
		send(TamagoEvent::kWokeUp, 0);
	send(TamagoEvent::kChangedType, (int32)convertTamagoTypeToVisualType(newType));
	simulateSleepCycle();
	return 1;
}

bool Tamago::hasGoneHome() const {
	return _goneHomeReason != TamagoGoneHomeReason::kNone;
}

bool Tamago::canBeWokenUp() const {
	if (_type == TamagoType::kEgg ||
		_type == TamagoType::kBabytchi ||
		hasGoneHome() ||
		_isInDaycare ||
		_isAwake)
		return false;
	return _lastSimulation.tm_hour >= 6 && _lastSimulation.tm_hour <= _sleepCycleEnd._hour;
}

bool Tamago::isSleepTime() const {
	int curMin = _lastSimulation.tm_min, curHour = _lastSimulation.tm_hour,
		startMin = _sleepCycleStart._minute, startHour = _sleepCycleStart._hour,
		endMin = _sleepCycleEnd._minute, endHour = _sleepCycleEnd._hour;
	if (!_hasSleepCycle || startHour == endHour)
		return curMin >= startMin && curMin < endMin;

	if (endHour > startHour)
		return (curHour == startHour && curMin >= startMin) ||
			(curHour > startHour && curHour < endHour) ||
			(curHour == endHour && curMin < endMin);

	// startHour > endHour as sleep cycle wraps around day
	return (curHour == startHour && curMin <= startMin) ||
		curHour > startHour ||
		curHour < endHour ||
		(curHour == endHour && curMin < startMin);
}

void Tamago::formatTimeDateToString(const TimeDateEx &tm, int32 stringId) {
	// originally this would be the system-default date and time format concattenated one after the other
	// instead we just output ISO 8601 without the milliseconds/timezone field and the separating T
	auto string = Common::String::format("%04d-%02d-%02d %02d:%02d:%02d",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	_engine->getScript()->setString(stringId, string);
}

int32 Tamago::formatDateToString(const TimeDateEx &tm, int32 stringId) {
	auto string = Common::String::format("%04d-%02d-%02d",
		tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	_engine->getScript()->setString(stringId, string);
	return (int32)string.size();
}

}

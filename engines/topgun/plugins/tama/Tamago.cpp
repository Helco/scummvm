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

static void syncBool(Common::Serializer &ser, bool &value) {
	byte asByte = value ? 1 : 0;
	ser.syncAsByte(asByte);
	value = asByte ? 1 : 0;
}

void Tamago::BasicTimer::saveLoadWithSerializer(Common::Serializer &ser) {
	syncBool(ser, _enabled);
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
	actionSetType(0);
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

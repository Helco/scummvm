#include "Tamago.h"

namespace TopGun {

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
	return kDaysPerMonth[mon] + isLeapYear;
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

Tamago::Tamago(TopGunEngine *engine) : _engine(engine) {
}

}

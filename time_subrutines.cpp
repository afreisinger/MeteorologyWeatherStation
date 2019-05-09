// 
// 
// 

#include "time_subrutines.h"
#include <Time.h>
#include <TimeLib.h>
#include <Wire.h>


/////////////////////////////////////////////////////////////////////////////////////
/*-------- Time subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
time_t EvaluateDaylightSavingTime(time_t result) {
	if (
		(
		(                                                                  // plati od
			(month(result) == DAYLIGHTSAVING_START_MONTH)                      // breznove
			&& (month(nextSunday(result)) == (DAYLIGHTSAVING_START_MONTH + 1))  // posledni nedele
			&&
			(
			(weekday(result) != 1)
				||
				(hour(result) >= DAYLIGHTSAVING_START_HOUR)                      // od dvou hodin rano
				)
			)
			|| (month(result) > DAYLIGHTSAVING_START_MONTH)                 // a v nasledujich mesicich
			)
		&&
		(
		(                                                                  // plati do
			(month(result) == DAYLIGHTSAVING_STOP_MONTH)                       // rijnove
			&& (month(nextSunday(result)) == DAYLIGHTSAVING_STOP_MONTH)       // posledni nedele
			)
			||
			(
			(month(result) == DAYLIGHTSAVING_STOP_MONTH)
				&& (month(nextSunday(result)) == (DAYLIGHTSAVING_STOP_MONTH + 1))
				&& (weekday(result) == 1)
				&& (hour(result) < DAYLIGHTSAVING_STOP_HOUR)
				)
			|| (month(result) < DAYLIGHTSAVING_STOP_MONTH)                  // a v predchozich mesicich
			)
		) {
		result = result + DAYLIGHTSAVING_TIMESHIFT * SECS_PER_HOUR;
		TIME_ZONE = TIME_ZONE_DST;
	}
	else {
		TIME_ZONE = TIME_ZONE_NODST;
	};
	return result;
};

byte decToBcd(byte val) {/////////////////////////////////////////////////////////////////////////////////////
	return ((val / 10 * 16) + (val % 10));
};
byte bcdToDec(byte val)
{
	return ((val / 16 * 10) + (val % 16));
};

bool set_rtc_datum() {/////////////////////////////////////////////////////////////////////////////////////
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(3);//set register to day
	Wire.write(decToBcd(rtc_weekday));
	Wire.write(decToBcd(rtc_day));
	Wire.write(decToBcd(rtc_month));
	Wire.write(decToBcd(rtc_year - 2000));
	if (Wire.endTransmission() == 0) {
		return true;
	}
	else {
		return false;
	};
};
bool set_rtc_time() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(0);//set register to time
	Wire.write(decToBcd(rtc_second));
	Wire.write(decToBcd(rtc_minute));
	Wire.write(decToBcd(rtc_hour));
	if (Wire.endTransmission() == 0) {
		return true;
	}
	else {
		return false;
	};
};
bool get_rtc_datum() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(3);//set register to day
	Wire.endTransmission();
	if (Wire.requestFrom(DS3231_ADDRESS, 4) == 4) { //get 4 bytes(day,date,month,year);
		rtc_weekday = bcdToDec(Wire.read());
		rtc_day = bcdToDec(Wire.read());
		rtc_month = bcdToDec(Wire.read());
		rtc_year = bcdToDec(Wire.read()) + 2000;
		RTCworking = true;
		return true;
	}
	else {
		RTCworking = false;
		return false;
	};
};
bool get_rtc_time() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(0);//set register to time
	Wire.endTransmission();
	if (Wire.requestFrom(DS3231_ADDRESS, 3) == 3) { //get 3 bytes (seconds,minutes,hours);
		rtc_second = bcdToDec(Wire.read() & 0x7f);
		rtc_minute = bcdToDec(Wire.read());
		rtc_hour = bcdToDec(Wire.read() & 0x3f);
		return true;
	}
	else {
		return false;
	};
};
float get_rtc_temperature() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(17);//set register to DS3132 internal temperature sensor
	Wire.endTransmission();
	if (Wire.requestFrom(DS3231_ADDRESS, 2) == 2) {
		float ttc = (float)(int)Wire.read();
		byte portion = Wire.read();
		if (portion == 0b01000000) ttc += 0.25;
		if (portion == 0b10000000) ttc += 0.5;
		if (portion == 0b11000000) ttc += 0.75;
		return ttc;
	}
	else {
		return MAX_TEMPERATURE_EXCEED;
	};
};
bool read_rtc(time_t& cas) {
	if (get_rtc_datum()) {
		get_rtc_time();
		tm.Year = rtc_year - 1970;
		tm.Month = rtc_month;
		tm.Day = rtc_day;
		tm.Hour = rtc_hour;
		tm.Minute = rtc_minute;
		tm.Second = rtc_second;
		cas = makeTime(tm);
		return true;
	}
	else {
		return false;
	};
};
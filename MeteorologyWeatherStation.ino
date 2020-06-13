/////////////////////////////////////////////////////////////////////////////////////
// 
//	Adri�n Freisinger, Buenos Aires(AR), August 2018
//  Version 1.1.8.18
//
// afreisinger@gmail.com
//
// IoT Weather Station Temperature and Humedity Sensor
// Update the RTC DS3231 from NTP server by W5100 (optional) or  ESP8266 (future)
// Display on serial monitor, temperature and humedity from RTC control sensor, local DHT22 (optional), TMP36 (optional) and remote DHT22 via nRF2400 (future)
// Display on TFT 1.8" ST7735 (optional) or Nextion Display 4.3 (future)
//
// 
// HARDWARE:
// Arduino Mega - mandatory
// RTC DS3231	- mandatory
// Ethernet Shield W5100 - optional
// LCD 16x2 + KeyPad - optional , https://www.dfrobot.com/wiki/index.php/LCD_KeyPad_Shield_For_Arduino_SKU:_DFR0009
// TFT 1.8" ST7735 - optional
// SD Card	- optional
// DHT22 - optional
// TMP36 - optional (attached to A2)
// nRF24 remote sensor - future
// Nextion Display 4.3" - future
// Esp 8266 - future
// Node Red and mqtt support - future
// SD CArd datalogger support -future
// Upgrade to monitoring six remote sensors future
// 
//-----------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////


#include <Arduino.h>
#include <Time.h>			//Library Time.zip downloaded from http://www.pjrc.com/teensy/td_libs_Time.html
#include <TimeLib.h>
//#include <DigitalIO.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
//#include <SdFat.h>
#include <LiquidCrystal.h>

#include "control_subrutines.h"
#include "ether_subrutines.h"
#include "lcd_subrutines.h"



//#include <RF24.h>


//#include "radio_subrutines.h"
#include "globals.h"
#include "output_subrutines.h"
#include "flashrom_subrutines.h"
#include "flashrom_msg.h"
#include "sdlib.h"
//#include "sd_subrutines.h"
//#include "time_subrutines.h"


/* uncomment next line if ethernet shield is present */
#define ETHERNET_IS_PRESENT

/* uncomment next line if NRF24  is present  */
//#define NRF24_IS_PRESENT

/* uncomment next line if SD Card  is present  */
//#define SD_IS_PRESENT

/* uncomment next line if LinkSprite 16x2 LCD Keypad Shield is present  */
#define LCD16x2_IS_PRESENT

/* uncomment next line if I2C TFT 1.8" ST7735 is present */
//#define ADAFRUIT_ST7735_IS_PRESENT

/* uncomment next line if DHT22 is present */
#define DHT22_IS_PRESENT

/* uncomment next line if two TMP36 are connected */
//#define TMP36_IS_PRESENT

/* uncomment next line if updating time in the RTC chip is allowed */
#define ENABLE_RTC_UPDATE

#if defined(ETHERNET_IS_PRESENT)
//#include <Dhcp.h>
//#include <Dns.h>
#include <Ethernet.h>
//#include <EthernetClient.h>
//#include <EthernetServer.h>
#include <EthernetUdp.h>

/////////////////////////////////////////////////////////////////////////////////////
// Find the nearest server
// http://www.pool.ntp.org/zone/
// or
// http://support.ntp.org/bin/view/Servers/StratumTwoTimeServers

//IPAddress timeServer(10, 0, 1, 250);				// local time server
IPAddress timeServer(200, 1, 116, 29);				// AR global time server
/////////////////////////////////////////////////////////////////////////////////////
EthernetUDP Udp;
byte MACAddress[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xFE };
#define LOCALUDPPORT 8888
#define SDCARD_CS 4										// Chip Select SD Card on W5100  - HIGH Deactive DOWN Active
#define ETHERNET_CS 10									// Chip Select Ethernet on W5100 HIGH Deactive DOWN Active
#endif
/////////////////////////////////////////////////////////////////////////////////////
#define NTP_CHECK_PERIOD 3600 // 3600
#define NTP_RETRY_CHECK_PERIOD 60 //60
#define REPORTDS_TIME_PERIOD 1 //60
volatile bool ntp_interrupt_in_progress = false;
volatile bool NTPworking = false;
volatile bool NTPsetstime = false;
volatile bool NTPasked = false;
volatile bool NTPavailable = true;
/////////////////////////////////////////////////////////////////////////////////////
#if !defined(ETHERNET_IS_PRESENT) && defined(LCD16x2_IS_PRESENT)
#define DHCP_RETRY_PERIOD 0
#define LCD_UPDATE_PERIOD 0
#define D4		4	//	4	// LCD
#define D10		10	//	10 // Contraste
#endif

#if defined(ETHERNET_IS_PRESENT) && !defined(LCD16x2_IS_PRESENT)
#define DHCP_RETRY_PERIOD 300
#define LCD_UPDATE_PERIOD 1
#endif

#if !defined(ETHERNET_IS_PRESENT) && !defined(LCD16x2_IS_PRESENT)
#define DHCP_RETRY_PERIOD 60
#define LCD_UPDATE_PERIOD 1
#endif

#if defined(ETHERNET_IS_PRESENT) && defined(LCD16x2_IS_PRESENT)
// On the Ethernet shield, pin 10 is used to select the W5100 chip and pin 4 for the SD card.
// On the LCD shield, pin 10 is used for contrast and pin 4 as D4.
// We redefine and wire the lcd out of the ethernet shield or we twist pins.
// We use pin 30 and pin 12 for the LCD

#define DHCP_RETRY_PERIOD 300  //300
#define LCD_UPDATE_PERIOD 1
#define D4		30  // LCD data
#define D10		12	// LCD Contrast (PWM)
#endif
/////////////////////////////////////////////////////////////////////////////////////
#if defined(LCD16x2_IS_PRESENT)
#include <LiquidCrystal.h>
//#include <LCDKeypad.h>
#define RS		8		
#define EN		9				
#define D5		5		
#define D6		6		
#define D7		7
#define CONSTRAST	50
#define COL		16
#define	ROW		2
LiquidCrystal lcd (RS, EN, D4, D5, D6, D7);						// Constructor
bool ControlShowMe = true;
#endif
/////////////////////////////////////////////////////////////////////////////////////
#if defined(DHT22_IS_PRESENT)
#include <DHT_U.h>
#include <DHT.h>

#define TEMPERATURE_SENSOR_DHT22 1
#define TEMPERATURE_SENSOR_PIN	42
#define TEMPERATURE_SENSOR_TYPE DHT22
DHT dht(TEMPERATURE_SENSOR_PIN, TEMPERATURE_SENSOR_TYPE);       // Constructor

#endif
/////////////////////////////////////////////////////////////////////////////////////
#define TIME_SYSTEM_CLOCK              0b0000000000000001
#define TIME_RTC_CLOCK                 0b0000000000000010
#define TIME_FORMAT_SMTP_DATE          0b0000000000000100
#define TIME_FORMAT_SMTP_MSGID         0b0000000000001000
#define TIME_FORMAT_HUMAN              0b0000000000010000
#define TIME_FORMAT_HUMAN_SHORT        0b0000000000100000
#define TIME_FORMAT_TIME_ONLY          0b0000000001000000
#define TIME_FORMAT_TIME_ONLY_WO_COLON 0b0000000010000000
#define TIME_FORMAT_DATE_ONLY          0b0000000100000000
#define TIME_FORMAT_DATE_SHORT         0b0000001000000000

#define DAYLIGHTSAVING_START_MONTH 1 // 
#define DAYLIGHTSAVING_START_HOUR  1 // 2:00:00 -> 3:00:00
#define DAYLIGHTSAVING_STOP_MONTH  1 // 
#define DAYLIGHTSAVING_STOP_HOUR   1 // 3:00:00 -> 2:00:00
#define DAYLIGHTSAVING_TIMESHIFT   0 // Desplazamiento
#define TIME_ZONE_INT -3			 // Time Zone
#define TIME_ZONE_NODST "+0000"
#define TIME_ZONE_DST "+0000"
String TIME_ZONE = TIME_ZONE_NODST;
time_t time_dhcp_retry = 0;
time_t time_last_ntp_check = 0;
time_t time_last_reportDS = 0;
time_t time_last_lcd = 0;
time_t rtc_now = 0;
time_t rtc_epoch = 0;
tmElements_t tm;
/////////////////////////////////////////////////////////////////////////////////////
#define DS3231_ADDRESS 0x68 // decimal 104
int rtc_second;			//00-59;
int rtc_minute;			//00-59;
int rtc_hour;			//1-12 - 00-23;
int rtc_weekday;		//1-7
int rtc_day;			//01-31
int rtc_month;			//01-12
int rtc_year;			//0-99 + 2000;
#define TEMPERATURE_SENSOR_DS3231  0	// RTC Temperature sensor
#define TEMPERATURE_SENSOR_TMP36_1 A7   // TMP36 inside
//#define TEMPERATURE_SENSOR_TMP36_2 A3   // TMP36 outside
#define MAX_TEMPERATURE_EXCEED 111.11   // Celsius, bad sensor if exceed
bool RTCworking = false;
/////////////////////////////////////////////////////////////////////////////////////
#define BAUDRATE 9600
/////////////////////////////////////////////////////////////////////////////////////

long lastMsg = 0;
char msg[50];
int value = 0;
char temp[50];
char hum[50];


EthernetClient ethClient;



#if defined(SD_IS_PRESENT)
	
	File webFile, configFile;               // El fichero de la pagina web en la SD card
	File dataDebugging;
	File dataLog;
	File dataFile;
	
#endif
	String myDataStr = "";





struct package
{
	float temperature_dht = 25.0;
	float humidity_dht = 60;
	float temperature_rtc = 24.0;
};

//typedef struct package myData;








/************ Progress Bar ************/

#define lenght_bar 16.0			//Longitud de de la barra, total 16 caracteres

double percent = 100.0;
unsigned char b;
unsigned int piece;

/************ Progess Bar ************/





void setup() {
	int count;
	count = 0;
	// ...

	struct package data;
	char buffer[16];

	myDataStr = dtostrf(data.humidity_dht, 3, 0, buffer);
	
	pinMode(53, OUTPUT);
	
	

	
	Serial.begin(BAUDRATE, SERIAL_8N1);

	//MySerialPrint("\r\n"+ GetTextFromFlashMemory(PROJECT_MESSAGE) + "\r\n");
	MySerialPrint(GetTextFromFlashMemory(VERSION_MESSAGE) + "\r\n");
	MySerialPrint(GetTextFromFlashMemory(AUTHOR_MESSAGE) + ", " + GetTextFromFlashMemory(DATE_MESSAGE) + "\r\n");
	MySerialPrint(GetTextFromFlashMemory(EMAIL_MESSAGE) + "\r\n");
	MySerialPrint("\r\n" + GetTextFromFlashMemory(COPYRIGHT_MESSAGE) + "\r\n");


	Wire.begin();
		
#if defined(LCD16x2_IS_PRESENT)
	
	lcd.begin(COL,ROW);
	lcd.clear();
	lcd.createChar(0, BLOCK_1x8);			// 1/5 full block, progress bar effect
	lcd.createChar(1, BLOCK_2x8);			// 2/5 full block
	lcd.createChar(2, BLOCK_3x8);			// 3/5 full block
	lcd.createChar(3, BLOCK_4x8);			// 4/5 full block
	lcd.createChar(4, BLOCK_5x8);			// full block
	lcd.createChar(5, GRADO);				// �
	#if defined(ETHERNET_IS_PRESENT)		// Wire hacking
		pinMode(D10, OUTPUT);
		analogWrite(D10, CONSTRAST);
	#endif
#endif
	
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
#define LCDbacklightPin 3
	pinMode(LCDbacklightPin, OUTPUT);
	//digitalWrite(LCDbacklightPin, HIGH);
	analogWrite(LCDbacklightPin, 255);
	delay(5000);

	tft.initR(INITR_BLACKTAB);
	//tft.initR(INITR_GREENTAB);
	//tft.initR(INITR_REDTAB);

	tft.setRotation(1);
	tft.setTextWrap(true);
	tft.fillScreen(ST7735_BLACK);
	tft.setTextColor(ST7735_RED, ST7735_YELLOW);
	tft.setTextSize(3);
	tft.setCursor(8, 50);
	tft.println("RTC test");
	tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
	tft.setTextSize(1);
	tft.setCursor(0, 0);
	tft.println(GetTextFromFlashMemory(WELCOME_MESSAGE));
#endif
	
#if defined(TMP36_IS_PRESENT)
	 pinMode(TEMPERATURE_SENSOR_TMP36_1, INPUT);
   //pinMode(TEMPERATURE_SENSOR_TMP36_2, INPUT);
#endif

#if defined(DHT22_IS_PRESENT)
	dht.begin();
#endif

#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
	tft.setTextSize(2);
	tft.setCursor(0, 80);
#endif

	
#if defined(LCD16x2_IS_PRESENT)
	MyDisplayPrint(lcd, GetTextFromFlashMemory(PROJECT_MESSAGE), 0, 0);
	MyDisplayPrint(lcd, GetTextFromFlashMemory(VERSION_MESSAGE), 0, 1);
	delay(5000);
	MyDisplayPrint(lcd, GetTextFromFlashMemory(CLEAR_MESSAGE), 0, 0);
	MyDisplayPrint(lcd, GetTextFromFlashMemory(CLEAR_MESSAGE), 0, 1);
#endif

	time_t rtc_uninitialized_time = 0;
	if (read_rtc(rtc_uninitialized_time)) {setTime(rtc_uninitialized_time);};

	
	MySerialPrint(GetTextFromFlashMemory(UNINITIALIZED_RTC_MESSAGE) + " " + ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, rtc_uninitialized_time) + " " + ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, false) + "\r\n" + "\r\n");
	
	
	
//#if defined(SD_IS_PRESENT)

	
	File dataFile, dataLog;

	ChipSelect(ETHERNET_CS, false);     // Ethernet not active
	ChipSelect(SDCARD_CS, true);		// SD Card active

	    
	if (sdlib::InitializingSDCard(SDCARD_CS)) 			// Initializing Card
	{
		sdlib::InfoSDCard();							// Info SD Card
		sdlib::SpaceOnSDCard();							// Free Space on SD Card
		
		SD.begin(SDCARD_CS);
		
		dataFile = SD.open("debug.log", FILE_WRITE);			// logfile reboot event
		dataLog  = SD.open("datalog.dat", FILE_WRITE);			// logfile data event
		
		sdlib::LogToSDCard("\r\n" + ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN_SHORT, 0) + "::" + GetTextFromFlashMemory(PROJECT_MESSAGE) +"::"+ GetTextFromFlashMemory(VERSION_MESSAGE) +"::"+ GetTextFromFlashMemory(DATE_MESSAGE)+"\r\n", dataFile);
		
		dataFile.close();
		//dataLog.close();
	}

//#else
		// disable SD card
		pinMode(SDCARD_CS, OUTPUT);
		digitalWrite(SDCARD_CS, HIGH);
//#endif

}


void loop() {
	

	if (checkdhcp()) {
		while (true) {
			if (checkntp()) {
				time_last_ntp_check = now();
				NTPavailable = true;

			}
			else {
				
				NTPavailable = false;
				time_last_ntp_check = now() - NTP_CHECK_PERIOD + NTP_RETRY_CHECK_PERIOD;
				//time_last_ntp_check = now();
				//Serial.println("ACA");
				
			};
			
			while(time_last_ntp_check + NTP_CHECK_PERIOD > now()) {
				//Serial.println("ACAW");
				//delay(1000);
				ShowTime(false);
				//Serial.println("ACAS");
				//delay(1000);

				MaintainTimeSources(false); //if NTP server was asked by system time updater, show results
				//LogToSDCard(myDataStr + ",", dataLog);
				
				//dataLog.write((const uint8_t *)&myData, sizeof(myData));

				//Serial.println("ACAM");
			//	delay(1000);


			};
		};
	}
	else 
	{
		if (rtc_epoch == 0) {
			if (read_rtc(rtc_epoch)) {
				setTime(rtc_epoch);
			};
		};
		time_dhcp_retry = now();
		bool FirstTime = true;
		while (time_dhcp_retry + DHCP_RETRY_PERIOD >= now()) {
			ShowTime(FirstTime);
			
			if (FirstTime) {
				FirstTime = false;
			};
		};
	};

	}

	

void SwitchRfEth()
{	//added it before and after calling nRF;

	//Switch ethernet's CS pin state
	digitalWrite(10, !digitalRead(10));
	//Switch radio's CS pin state
	digitalWrite(53, !digitalRead(53));
}



#if defined(LCD16x2_IS_PRESENT)
void progressBar() {

	lcd.setCursor(0, 1);
	percent = value / 1024.0 * 100;
	Serial.print("percent:");
	Serial.println(percent);

	double a = lenght_bar / 100 * percent;

	// drawing black rectangles on LCD

	if (a >= 1) {
		for (int i = 1; i < a; i++) {
			lcd.write(4);
			b = i;
		}
		a = a - b;
	}
	piece = a * 5;
	// drawing charater's colums
	switch (piece) {

	case 0:
		break;
	case 1:
		lcd.print((char)0);
		//lcd.write(0);
		break;
	case 2:
		lcd.write(1);
		break;
	case 3:
		lcd.write(2);
		break;
	case 4:
		lcd.write(3);
		break;
	}

	//clearing line
	for (int i = 0; i < (lenght_bar - b); i++) {
		lcd.print(" ");
	};
}
#endif

/////////////////////////////////////////////////////////////////////////////////////
/*-------- Time subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
time_t EvaluateDaylightsavingTime(time_t result) {
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
			|| (month(result) > DAYLIGHTSAVING_START_MONTH)                 // 	
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
#if defined(DHT22_IS_PRESENT)
float get_dht_temperature() {
	float  t = dht.readTemperature();
	return t;
}
float get_dht_humedity() {
	float  h = dht.readHumidity();
	return h;
}
#endif
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
/////////////////////////////////////////////////////////////////////////////////////
/*-------- PrintTime subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
String ComposeZerosLeadedNumber(unsigned long int MyNumber, byte NumberOfCharacters) {
	String TempString = "";
	for (byte index = 1; index <= NumberOfCharacters; index++) {
		TempString = "0" + TempString;
	};
	TempString = TempString + String(MyNumber);
	int ifrom = TempString.length() - NumberOfCharacters;
	int ito = TempString.length();
	return TempString.substring(ifrom, ito);
};
String ComposeTimeStamp(unsigned int details, time_t datum) {
	String strday;
	String strmonth;
	String stryear;
	String strshortyear;
    String strhour;
	String strminute;
	String strsecond;
	String strweekday;
	int myvalue;
	if (
		((details & TIME_RTC_CLOCK) != TIME_RTC_CLOCK)
		&&
		((details & TIME_SYSTEM_CLOCK) != TIME_SYSTEM_CLOCK)
		) {
		strday = GetTextFromFlashMemory(TIMESOURCE_ERRROR_MESSAGE);
		return strday;
	};
	if ((details & TIME_RTC_CLOCK) == TIME_RTC_CLOCK) {
		if (datum <= 0) {
			if (!read_rtc(datum)) {
				if (NTPworking) {
					strday = GetTextFromFlashMemory(RTC_ERROR_MESSAGE);
				}
				else {
					strday = GetTextFromFlashMemory(TIME_ERRROR_MESSAGE);
				};
				return strday;
			};
		};
	};
	if ((details & TIME_SYSTEM_CLOCK) == TIME_SYSTEM_CLOCK) {
		if (datum <= 0) {
			datum = now();
		};
	};
	datum = EvaluateDaylightsavingTime(datum);
	strday = ComposeZerosLeadedNumber(day(datum), 2);
	myvalue = month(datum);
	if ((details & TIME_FORMAT_SMTP_DATE) == TIME_FORMAT_SMTP_DATE) {
		strmonth = monthShortStr(myvalue);
	}
	else {
		strmonth = ComposeZerosLeadedNumber(myvalue, 2);
	};
	stryear = String(year(datum));
	strshortyear = String(year(datum) - 2000);
	strhour = ComposeZerosLeadedNumber(hour(datum), 2);
	strminute = ComposeZerosLeadedNumber(minute(datum), 2);
	strsecond = ComposeZerosLeadedNumber(second(datum), 2);
	int tweekday;
	if (details & TIME_RTC_CLOCK) {
		tweekday = rtc_weekday;
	}
	else {
		tweekday = weekday(datum);
	};
	if ((details & TIME_FORMAT_HUMAN) == TIME_FORMAT_HUMAN) {
		strweekday = dayStr(tweekday);
	}
	else {
		strweekday = dayShortStr(tweekday);//dayShortStr(tweekday);
	};
	if ((details & TIME_FORMAT_HUMAN) == TIME_FORMAT_HUMAN) {
		return String(strday + "." + strmonth + "." + stryear + " " + strhour + ":" + strminute + ":" + strsecond + " " + strweekday);
	}
	else {
		if ((details & TIME_FORMAT_HUMAN_SHORT) == TIME_FORMAT_HUMAN_SHORT) {
			return String(strday + "." + strmonth + "." + stryear + " " + strhour + ":" + strminute + ":" + strsecond + " "); //20 characters for LCD
		}
		else {
			if ((details & TIME_FORMAT_TIME_ONLY) == TIME_FORMAT_TIME_ONLY) {
				return String(strhour + ":" + strminute + ":" + strsecond);
			}
			else {
				if ((details & TIME_FORMAT_TIME_ONLY_WO_COLON) == TIME_FORMAT_TIME_ONLY_WO_COLON) {
					return String(strhour + " " + strminute + " " + strsecond);
				}
				else {
					if ((details & TIME_FORMAT_DATE_ONLY) == TIME_FORMAT_DATE_ONLY) {
						return String(strday + "." + strmonth + "." + stryear);
					}
					else {
						if ((details & TIME_FORMAT_SMTP_MSGID) == TIME_FORMAT_SMTP_MSGID) {
							return String(stryear + strmonth + strday + strhour + strminute + strsecond + ".") + String(millis());
						}
						else {
							if ((details & TIME_FORMAT_DATE_SHORT) == TIME_FORMAT_DATE_SHORT) {
								return String(strweekday + " " + strday + "/" + strmonth + "/" + strshortyear);
							}
						
							else {
								//return String(strweekday + ", " + strday + ". " + strmonth + "." + stryear + " " + strhour + ":" + strminute + ":" + strsecond + " " + TIME_ZONE);
								return String(" " + strweekday + " " + strday + "/" + strmonth + "/" + stryear);

							};
						};
					};
				};
			};
		};
	};
};

float ComposeTemperatureValue(byte sensor) {
	String TempStr = "";
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float TemperatureC1, TemperatureC2;
	if (sensor == TEMPERATURE_SENSOR_DS3231) {
		 TemperatureC1 = get_rtc_temperature();
		//TemperatureC1 = get_dht_temperature();
	}
	
#if defined(DHT22_IS_PRESENT)
	else if (sensor == TEMPERATURE_SENSOR_DHT22)
	{
		TemperatureC1 = get_dht_temperature();
	}
#endif
	else														
	{
		TemperatureC1 = (((analogRead(sensor) * (5000.0 / 1024)) - 500) / 10) + 25;
			
	}
		return TemperatureC1;
};







float ComposeHumedityValue(byte sensor) {
	float HumedityH1;
#if defined(DHT22_IS_PRESENT)
		if (sensor == TEMPERATURE_SENSOR_DHT22)
	{
			HumedityH1 = get_dht_humedity();
	}
#endif
		return HumedityH1;
}

////////////////////////////////////////////////////////////////////////////////////
String ComposeHumedityString(byte sensor, bool displaytype) {
	
	String HumStr = "";
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float HumedityH = ComposeHumedityValue(sensor);
	if (HumedityH < 100) {
		char buffer[16];
	//HumStr = dtostrf(HumedityH, 5, 2, buffer);
	  HumStr = dtostrf(HumedityH, 3, 0, buffer);
		if (displaytype) {
			HumStr = HumStr + "%"; //oC for Asian Character Set
		}
		else {
			HumStr = HumStr + "%"; //oC for Western Character Set
		};
	}
	else {
		//error message
		HumStr = GetTextFromFlashMemory(TEMPERATURE_ERRROR_MESSAGE);
	};
	return HumStr;
};
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
String ComposeTemperatureString(byte sensor, bool displaytype) {
	String TempStr = "";
	
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float TemperatureC = ComposeTemperatureValue(sensor);
	if (TemperatureC < MAX_TEMPERATURE_EXCEED) {
		char buffer[16];
		TempStr = dtostrf(TemperatureC, 5, 2, buffer);
		if (displaytype) {
			//TempStr = TempStr + "\xDF\x43"; //oC for Asian Character Set
			TempStr = TempStr;
		}
		else {
			TempStr = TempStr + "\xF8\x43"; //oC for Western Character Set
			 
		};
	}
	else {
		//error message
		TempStr = GetTextFromFlashMemory(TEMPERATURE_ERRROR_MESSAGE);
	};
	return TempStr;
};
////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////
/*-------- NTP subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
#define NTP_PACKET_SIZE 48 // NTP time is in the first 48 bytes of message
/////////////////////////////////////////////////////////////////////////////////////
time_t getNtpTime() {

	if (ntp_interrupt_in_progress == false) {
		ntp_interrupt_in_progress = true;
		Serial.println(GetTextFromFlashMemory(NTP_SEND_MESSAGE));
		while (Udp.parsePacket() > 0); // discard any previously received packets
		byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets 
		memset(packetBuffer, 0, NTP_PACKET_SIZE);
		packetBuffer[0] = 0b11100011;   // LI, Version, Mode
		packetBuffer[1] = 0;     // Stratum, or type of clock
		packetBuffer[2] = 6;     // Polling Interval
		packetBuffer[3] = 0xEC;  // Peer Clock Precision
		packetBuffer[12] = 49;
		packetBuffer[13] = 0x4E;
		packetBuffer[14] = 49;
		packetBuffer[15] = 52;
		Udp.beginPacket(timeServer, 123); //NTP requests are to port 123
		Udp.write(packetBuffer, NTP_PACKET_SIZE);
		Udp.endPacket();
		delay(1500);
		volatile time_t result = 0;
		volatile int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			volatile unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			result = secsSince1900 - 2208988800UL + TIME_ZONE_INT * SECS_PER_HOUR;
			if (
				(
				(                                                                  
					(month(result) == DAYLIGHTSAVING_START_MONTH)                     
					&& (month(nextSunday(result)) == (DAYLIGHTSAVING_START_MONTH + 1)) 
					&&
					(
					(weekday(result) != 1)
						||
						(hour(result) >= DAYLIGHTSAVING_START_HOUR)                      
						)
					)
					|| (month(result) > DAYLIGHTSAVING_START_MONTH)                
					)
				&&
				(
				(                                                                  
					(month(result) == DAYLIGHTSAVING_STOP_MONTH)                       
					&& (month(nextSunday(result)) == DAYLIGHTSAVING_STOP_MONTH)       
					)
					||
					(
					(month(result) == DAYLIGHTSAVING_STOP_MONTH)
						&& (month(nextSunday(result)) == (DAYLIGHTSAVING_STOP_MONTH + 1))
						&& (weekday(result) == 1)
						&& (hour(result) < DAYLIGHTSAVING_STOP_HOUR)
						)
					|| (month(result) < DAYLIGHTSAVING_STOP_MONTH)                 
					)
				) {
				result = result + DAYLIGHTSAVING_TIMESHIFT * SECS_PER_HOUR;
				TIME_ZONE = TIME_ZONE_DST;
			}
			else {
				TIME_ZONE = TIME_ZONE_NODST;
			};
			NTPworking = true;
			Serial.print(GetTextFromFlashMemory(NTP_RESPONSES_MESSAGE));
			Serial.print(ComposeTimeStamp(TIME_SYSTEM_CLOCK | TIME_FORMAT_HUMAN, result));
			Serial.println();
			Serial.print(GetTextFromFlashMemory(RTC_TIME_MESSAGE));
			Serial.println(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, 0));
		}
		else {
			NTPworking = false;
			Serial.println(GetTextFromFlashMemory(NTP_NO_RESPONSE_MESSAGE));
		}
		ntp_interrupt_in_progress = false;
		return result;
	};
};
/////////////////////////////////////////////////////////////////////////////////////
bool checkntp()
{
	
	Serial.print(GetTextFromFlashMemory(NTP_WAIT_MESSAGE));
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		Serial.print(timeServer[thisByte], DEC);
		if (thisByte < 3) {
			Serial.print(F("."));
		};
	};
	Serial.println(F(" ..."));
	setTime(getNtpTime());
	get_rtc_datum();
	get_rtc_time();
	time_t avr_now = now();
	setTime(rtc_hour, rtc_minute, rtc_second, rtc_day, rtc_month, rtc_year);
	time_t rtc_now = now();
	setTime(avr_now);
	if (rtc_now == avr_now) {
		Serial.println(GetTextFromFlashMemory(RTC_OK_MESSAGE));
	}
	else {
		if (NTPworking) {
			rtc_second = second();
			rtc_minute = minute();
			rtc_hour = hour();
			rtc_weekday = weekday();
			rtc_day = day();
			rtc_month = month();
			rtc_year = year();
			if (!set_rtc_time()) {
				Serial.println(GetTextFromFlashMemory(RTC_TF_MESSAGE));
			}
			else {
				if (!set_rtc_datum()) {
					Serial.println(GetTextFromFlashMemory(RTC_DF_MESSAGE));
				}
				else {
					Serial.println(GetTextFromFlashMemory(RTC_SYNC_MESSAGE));
				};
			};
			return true;
		}
		else {
			Serial.println(GetTextFromFlashMemory(ERROR_MESSAGE));
			return false;
		};
	};
};
/////////////////////////////////////////////////////////////////////////////////////
/*-------- DHCP subroutine ----------*/
/////////////////////////////////////////////////////////////////////////////////////
bool checkdhcp() {
	
#if defined(ETHERNET_IS_PRESENT)
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.fillScreen(ST7735_BLACK);
	tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
	tft.setTextSize(1);
	tft.setCursor(0, 0);
#endif
	//PRINT PROBLEMA
	Serial.print("");
	MyPrint(GetTextFromFlashMemory(MAC_MESSAGE));
	MySerialPrint(" = ");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.println();
#endif
	for (byte thisByte = 0; thisByte < 6; thisByte++) {
		if (MACAddress[thisByte] < 16) {
			MyPrint("0");
		};
		String hexString = String(MACAddress[thisByte], HEX);
		hexString.toUpperCase();
		MyPrint(hexString);
		if (thisByte < 5) {
			MyPrint(":");
		};
	};
	MyPrintLn("");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.println();
#endif
	MyPrintLn(GetTextFromFlashMemory(DHCP_WAIT_MESSAGE));

#if defined(LCD16x2_IS_PRESENT)
	if (ControlShowMe) //One time show me
	{
		MyDisplayPrint(lcd, GetTextFromFlashMemory(DHCP_WAIT_MESSAGE), 0, 0);
		ControlShowMe = false;
		/*
		value = 8;
		for (byte index = 0; index <= 128; index++) {
		MyPrint(String(index) + " ");
		progressBar();
		value = value + 8;
		delay(100);
		};*/
	}
#endif // defined(LCD16x2_IS_PRESENT


	
	
	if (Ethernet.begin(MACAddress) == 0) {
		MyPrintLn(GetTextFromFlashMemory(DHCP_FAIL_MESSAGE));
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(10000);
		tft.fillScreen(ST7735_BLACK);
#endif
		return false;  // No ip Address
	}
	else {
		MyPrintLn(GetTextFromFlashMemory(DHCP_PASS_MESSAGE));
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(2000);
		tft.fillScreen(ST7735_BLACK);
		tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
		tft.setTextSize(1);
		tft.setCursor(0, 0);
#endif
		
		
		MyPrint(GetTextFromFlashMemory(IP_MESSAGE));
		MySerialPrint(" = ");

#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		
		#if defined(SD_IS_PRESENT)
			File dataFile = SD.open("debug.log", FILE_WRITE);
			LogToSDCard(GetTextFromFlashMemory(IP_MESSAGE) + " = ", dataFile);
		#endif


		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.localIP()[thisByte], DEC));
			#if defined(SD_IS_PRESENT) 
				LogToSDCard(String(Ethernet.localIP()[thisByte], DEC), dataFile); 
			#endif
			
			if (thisByte < 3) {
				MyPrint(".");
				#if defined(SD_IS_PRESENT)
					LogToSDCard(".", dataFile);
				#endif
			};
		};
		MyPrintLn("");
		#if defined(SD_IS_PRESENT)
			LogToSDCard("\r\n",dataFile);
			dataFile.close();
		#endif



#if defined(LCD16x2_IS_PRESENT)				
		MyDisplayPrint(lcd, GetTextFromFlashMemory(IP_MESSAGE), 0, 0);
		MyDisplayPrint(lcd, String(Ethernet.localIP()[0], DEC) +"."+ String(Ethernet.localIP()[1], DEC) +"."+ String(Ethernet.localIP()[2], DEC) + "." + String(Ethernet.localIP()[3], DEC) , 0, 1);
		
		delay(5000);
		lcd.clear();

#endif // defined(LCD16x2_IS_PRESENT

#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif

		MyPrint(GetTextFromFlashMemory(MASK_MESSAGE));
		MySerialPrint(" = ");

#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.subnetMask()[thisByte], DEC));
			if (thisByte < 3) {
				MyPrint(".");
			};
		};
		MyPrintLn("");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif

		
		#if defined(SD_IS_PRESENT)
		dataFile = SD.open("debug.log", FILE_WRITE);
		LogToSDCard(GetTextFromFlashMemory(GATEWAY_MESSAGE) + " = ", dataFile);
		#endif
		
		MyPrint(GetTextFromFlashMemory(GATEWAY_MESSAGE));
		MySerialPrint(" = ");

		for (byte thisByte = 0; thisByte < 4; thisByte++) 
			{
			MyPrint(String(Ethernet.gatewayIP()[thisByte], DEC));
			
			#if defined(SD_IS_PRESENT)
				LogToSDCard(String(Ethernet.gatewayIP()[thisByte], DEC), dataFile);
			#endif

			if (thisByte < 3)
			{
				MyPrint(".");
				#if defined(SD_IS_PRESENT)
				LogToSDCard(".", dataFile);
				#endif
			};
		};

		;
		MyPrintLn("");
		#if defined(SD_IS_PRESENT)
			LogToSDCard("\r\n", dataFile);
		dataFile.close();
		#endif


#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		Serial.print("");
		MyPrint(GetTextFromFlashMemory(DNS_MESSAGE));
		MySerialPrint(" = ");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.dnsServerIP()[thisByte], DEC));
			if (thisByte < 3) {
				MyPrint(".");
			};
		};
		MyPrint("\r\n");

		Udp.begin(LOCALUDPPORT);
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(10000);
		tft.fillScreen(ST7735_BLACK);
#endif
		return true;
	};
#else
	return false;
#endif
};



void ShowTime(bool ForceShowTemperature) {/////////////////////////////////////////////////////////////////////////////////////
	
	

	if (time_last_lcd + LCD_UPDATE_PERIOD <= now()) {
		time_last_lcd = now();
		String PrtString;
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
		tft.setTextSize(2);
		tft.setCursor(21, 0);
		PrtString = ComposeTimeStamp(TIME_SYSTEM_CLOCK bitor TIME_FORMAT_DATE_ONLY, 0) + "  ";
		tft.print(PrtString);

		tft.setTextSize(3);
		tft.setCursor(9, 53);
		PrtString = ComposeTimeStamp(TIME_SYSTEM_CLOCK bitor TIME_FORMAT_TIME_ONLY, 0);
		tft.print(PrtString);
#endif

		if ((time_last_reportDS + REPORTDS_TIME_PERIOD <= now()) || (ForceShowTemperature)) {
			time_last_reportDS = now();

#if defined(ADAFRUIT_ST7735_IS_PRESENT)
			tft.setTextSize(3);
			tft.setCursor(18, 107);
			PrtString = ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, false);
			tft.print(PrtString);
#endif
			//order to avoid distortion running NTP process console print is omited
			if (!NTPasked) {
				MySerialPrint(GetTextFromFlashMemory(INFO_MESSAGE));
				MySerialPrint(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, 0));
				MySerialPrint(" ");
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, false));
				

#if defined(TMP36_IS_PRESENT)
				MySerialPrint(" ");
				MySerialPrint(GetTextFromFlashMemory(SENSORS_MESSAGE));
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_TMP36_1, false));
				//MySerialPrint(" ");
				//MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_TMP36_2, false));
				
#endif
				

#if defined(DHT22_IS_PRESENT)
				
				int count;
				//MySerialPrint(" ");
				Serial.print(" ");
				MySerialPrint(GetTextFromFlashMemory(SENSORS_MESSAGE_2));
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_DHT22, false) +" "+ ComposeHumedityString(TEMPERATURE_SENSOR_DHT22,false));	
				#if defined(SD_IS_PRESENT)
					LogToSDCard(myDataStr + ",", dataLog);
				#endif

#endif
				MySerialPrint("\r\n");


#if defined(LCD16x2_IS_PRESENT) && defined(DHT22_IS_PRESENT)
				lcd.setCursor(0, 0);
				lcd.print(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_TIME_ONLY, 0) + " " + ComposeTemperatureString(TEMPERATURE_SENSOR_DHT22,true));
				lcd.setCursor(14, 0);
				lcd.write(5) + lcd.print("\x43");
				lcd.setCursor(0, 1);
				lcd.print(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_DATE_SHORT, 0) + ComposeHumedityString(TEMPERATURE_SENSOR_DHT22, false)); //TIME_FORMAT_DATE_SHORT
#endif

#if defined(LCD16x2_IS_PRESENT) && !defined(DHT22_IS_PRESENT)
				lcd.setCursor(0, 0);
				lcd.print(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_TIME_ONLY, 0) + " " + ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, true));
				lcd.setCursor(14, 0);
				lcd.write(5) + lcd.print("\x43");
				lcd.setCursor(0, 1);
				lcd.print(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_DATE_SHORT, 0)); //TIME_FORMAT_DATE_SHORT
#endif
		
				
				



			};
		};
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(490);
		tft.setTextSize(3);
		tft.setCursor(9, 53);
		PrtString = ComposeTimeStamp(TIME_SYSTEM_CLOCK bitor TIME_FORMAT_TIME_ONLY_WO_COLON, 0);
		tft.print(PrtString);
#endif
	};


	
};



bool  MaintainTimeSources(bool force) {/////////////////////////////////////////////////////////////////////////////////////
	
	String OutString = "";
	if ((NTPasked) || (force)) {
		NTPasked = false;
		time_t avr_now = 0;
		time_t timediff = 0;
		if (!read_rtc(rtc_now)) {
			rtc_now = 0;
		};
		avr_now = now();
		if (NTPworking) {
			MySerialPrint(GetTextFromFlashMemory(NTP_RESPONSES_MESSAGE));
			MySerialPrint(ComposeTimeStamp(TIME_SYSTEM_CLOCK | TIME_FORMAT_HUMAN, avr_now));
			MySerialPrint(", SYS Epoch: " + String(avr_now) + "\r\n");
		}
		else {
			MySerialPrint(GetTextFromFlashMemory(NTP_ERROR_MESSAGE) + "\r\n");
			return false;
		};
		if (RTCworking) {
			MySerialPrint(GetTextFromFlashMemory(RTC_TIME_MESSAGE));
			MySerialPrint(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, rtc_now));
			MySerialPrint(", RTC Epoch: " + String(rtc_now) + "\r\n");
			String strSign = "";
			if (rtc_now > avr_now) {
				timediff = rtc_now - avr_now;
				strSign = "+";
			}
			else {
				timediff = avr_now - rtc_now;
				strSign = "-";
			};
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
#define ST7735_SILVER		0xC618
#define ST7735_GRAY		0x8410
			tft.setTextSize(1);
			tft.setCursor(0, 79);
			tft.setTextColor(ST7735_BLACK, ST7735_BLACK);
			tft.print(LastOutString);
			tft.setCursor(0, 79);
			tft.setTextColor(ST7735_GRAY, ST7735_BLACK);
#endif
			if (timediff == 0) {
				OutString = GetTextFromFlashMemory(RTC_OK_MESSAGE);
				MyPrintLn(OutString);
			}
			else {
				OutString = GetTextFromFlashMemory(RTC_NEED_SYNC_MESSAGE) + strSign + String(timediff) + GetTextFromFlashMemory(SECONDS_MESSAGE);
				MyPrintLn(OutString);
#if defined(ENABLE_RTC_UPDATE)
				rtc_second = second();
				rtc_minute = minute();
				rtc_hour = hour();
				rtc_weekday = weekday();
				rtc_day = day();
				rtc_month = month();
				rtc_year = year();
				if (!set_rtc_time()) {
					MySerialPrint(GetTextFromFlashMemory(RTC_TF_MESSAGE) + "\r\n");
				}
				else {
					if (!set_rtc_datum()) {
						MySerialPrint(GetTextFromFlashMemory(RTC_DF_MESSAGE) + "\r\n");
					}
					else {
						MySerialPrint(GetTextFromFlashMemory(RTC_SYNC_MESSAGE) + "\r\n");
					};
				};
#endif    
			};
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
			tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
			LastOutString = OutString;
#endif
		}
		else {
			MySerialPrint(GetTextFromFlashMemory(RTC_NOT_PRESENT) + "\r\n");
		};
		return true;
	};
	return false;
};



/*

String GetTextFromFlashMemory(int ItemIndex)
{
	int i = 0;
	char c;
	while ((c != '\0') && (i < BUFFERSIZE))
	{
		c = pgm_read_byte(pgm_read_word(&TextItemPointers[ItemIndex]) + i);
		buffer[i] = c;
		i++;
	}
	buffer[i] = '\0';
	return String(buffer);
}

*/


// C runtime variables
// -------------------
extern unsigned int __bss_end;
extern unsigned int __heap_start;
extern void *__brkval;

/*!
@function   freeMemory
@abstract   Return available RAM memory
@discussion This routine returns the ammount of RAM memory available after
initialising the C runtime.
@param
@return     Free RAM available.
*/
static int freeMemory(void)
{
	int free_memory;

	if ((int)__brkval == 0)
		free_memory = ((int)&free_memory) - ((int)&__bss_end);
	else
		free_memory = ((int)&free_memory) - ((int)__brkval);

	return free_memory;
}



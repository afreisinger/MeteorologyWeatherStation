//#pragma once

#include "output_subrutines.h"
#include "LiquidCrystal.h"

/////////////////////////////////////////////////////////////////////////////////////
/*-------- Output  subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
//distribute output to two serial lines and LCD display

void MySerialPrint(String displaytext) {
	Serial.print(displaytext);
#if defined(SECOND_SERIAL_IS_PRESENT)
	Serial1.print(displaytext);
#endif
};

void MySerialPrintln(String displaytext) {
	Serial.println(displaytext);
#if defined(SECOND_SERIAL_IS_PRESENT)
	Serial1.println(displaytext);
#endif
};

void MyDisplayPrint(LiquidCrystal lcd, String displaytext, int col, int row) {
	lcd.setCursor(col, row);
	lcd.print(displaytext);
};




void MyPrint(String text) {
	MySerialPrint(text);
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.print(text);
#endif
};

void MyPrintLn(String text) {
	MySerialPrint(text + "\r\n");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.println(text);
#endif
};


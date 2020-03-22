// output_sub.h

#ifndef output_subrutines
#define output_subrutines

#include "Arduino.h"
#include "LiquidCrystal.h"

void MySerialPrint(String displaytext);
void MySerialPrintln(String displaytext);
void MyPrint(String text);
void MyPrintLn(String text);
void MyDisplayPrint(LiquidCrystal lcd, String displaytext, int col, int row);


#endif


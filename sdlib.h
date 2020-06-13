// 

#ifndef SDLIB_H
#define SDLIB_H

#include "Arduino.h"
#include "SPI.h"
#include "SD.h"
//#include "SdFat.h"
#include "flashrom_msg.h"
#include "flashrom_subrutines.h"

class sdlib {

public:
    static bool InitializingSDCard(uint8_t chipSelect);
    static void InfoSDCard();
    static void LogToSDCard(String var, File file);
    static void SpaceOnSDCard();
};
#endif


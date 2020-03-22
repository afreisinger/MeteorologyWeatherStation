// sd_subrutines.h

#ifndef _SD_SUBRUTINES_h
#define _SD_SUBRUTINES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif


bool InitializingSDCard(uint8_t chipselect);

void LogToSDCard(String var, File file);


#endif


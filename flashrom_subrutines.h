// flashrom_subrutines.h

#ifndef _FLASHROM_SUBRUTINES_h
#define _FLASHROM_SUBRUTINES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

String GetTextFromFlashMemory(int ItemIndex);

#endif


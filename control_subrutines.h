// control_subrutines.h

#ifndef _CONTROL_SUBRUTINES_h
#define _CONTROL_SUBRUTINES_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

void ChipSelect(uint8_t pin, bool enable);
#endif


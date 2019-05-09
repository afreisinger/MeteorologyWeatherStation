// 
// 
// 

#include "control_subrutines.h"



void ChipSelect(uint8_t pin, bool enable)
{
	if (enable)
		//digitalWrite(pin, !digitalRead(pin));  
	{	pinMode(pin, OUTPUT);
		digitalWrite(pin, LOW); // active
	}
	else
		//digitalWrite(pin, digitalRead(pin));   
	{
		pinMode(pin, OUTPUT);
		digitalWrite(pin, HIGH); // de active
	}
}
#include <Arduino.h>
#include <RF24.h>
#include "radio_subrutines.h"


//byte addresses[][6] = { "1Node","2Node" };
//byte address[5] = "00001";
byte address[][6] = { "1Node" }; // Create address for 1 pipe.

//Create up to 6 pipe addresses P0 - P5;  the "LL" is for LongLong type
const uint64_t remoteAddress[] = { 0x7878787878LL, 0xB3B4B5B6F1LL, 0xB3B4B5B6CDLL, 0xB3B4B5B6A3LL, 0xB3B4B5B60FLL, 0xB3B4B5B605LL };


void radio_setup(RF24 radio) {
	radio.begin();								//Start the nRF24 module.
	radio.setPALevel(RF24_PA_LOW);				// "short range setting" - increase if you want more range AND have a good power supply.
	radio.setDataRate(RF24_250KBPS);
	radio.setChannel(108);						// the higher channels tend to be more "open" 108 es the lastone.
	radio.openReadingPipe(1, address[0]); // Open up to six pipes for PRX to receive data
	//radio.openReadingPipe(1, remoteAddress[1]);
	//radio.openReadingPipe(2, remoteAddress[2]);
	//radio.openReadingPipe(3, remoteAddress[3]);
	//radio.openReadingPipe(4, remoteAddress[4]);
	//radio.openReadingPipe(5, remoteAddress[5]);
	radio.startListening();
};







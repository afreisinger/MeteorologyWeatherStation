// /////////////////////////////////////////////////////////////////////////////////////
/*-------- FLASH ROM subroutine ----------*/
///////////////////////////////////////////////////////////////////////////////////////


#include "flashrom_subrutines.h"

#define BUFFERSIZE 120
static char buffer[BUFFERSIZE + 1];


const char TextInfo0[] PROGMEM = "\r\n\xA9 2018, Freisinger \x8Aec\r\n\r\nGet NTP and RTC chip time\r\n=========================";
const char TextInfo1[] PROGMEM = "\r\nUninitialized RTC time and temperature:";
const char TextInfo2[] PROGMEM = "Waiting for DHCP lease ...";
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
const char TextInfo3[] PROGMEM = "Failed to configure IPv4  for using Dynamic Host    Configuration Protocol!";
const char TextInfo4[] PROGMEM = "Dynamic Host ConfigurationProtocol passed.";
#else
const char TextInfo3[] PROGMEM = "Failed to configure IPv4 for using Dynamic Host Configuration Protocol!";
const char TextInfo4[] PROGMEM = "Dynamic Host Configuration Protocol passed.";
#endif
const char TextInfo5[] PROGMEM = "Board IP address";
const char TextInfo6[] PROGMEM = "Network subnet mask";
const char TextInfo7[] PROGMEM = "Network gateway IP address";
const char TextInfo8[] PROGMEM = "DNS IP address";
const char TextInfo9[] PROGMEM = "\r\nThe network interface MAC address ";
const char TextInfo10[] PROGMEM = "Waiting to obtain the accurate time from NTP server ";
const char TextInfo11[] PROGMEM = "Sending NTP request... ";
const char TextInfo12[] PROGMEM = "and NTP responses.\r\nNTP time: ";
const char TextInfo13[] PROGMEM = "No NTP Response.";
const char TextInfo14[] PROGMEM = "Failed to get the current time from NTP server.";
const char TextInfo15[] PROGMEM = "RTC time: ";
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
const char TextInfo16[] PROGMEM = " RTC chip do not need to   be updated.";
const char TextInfo17[] PROGMEM = "RTC need to be updated.   Current time difference is";
#else
const char TextInfo16[] PROGMEM = "RTC chip do not need to be updated.";
const char TextInfo17[] PROGMEM = "RTC need to be updated. Current time difference is ";
#endif
const char TextInfo18[] PROGMEM = " second(s).";
const char TextInfo19[] PROGMEM = "RTC chip has been successfully updated.";
const char TextInfo20[] PROGMEM = "Error setting time to RTC.";
const char TextInfo21[] PROGMEM = "Error setting date to RTC.";
const char TextInfo22[] PROGMEM = "RTC Time and Temperature: ";
const char TextInfo23[] PROGMEM = "Failed to get the current time from RTC chip.";
const char TextInfo24[] PROGMEM = "No data (temperature sensor error).";
const char TextInfo25[] PROGMEM = "Current time failed to get either the NTP server or the RTC chip.";
const char TextInfo26[] PROGMEM = "TMP36: ";
const char TextInfo27[] PROGMEM = "Unable to compare NTP and RTC time (RTC is not present?)";
const char TextInfo28[] PROGMEM = "Error calling function: function ComposeTimeStamp requires time source (CPU or RTC).";
const char TextInfo29[] PROGMEM = "Error getting time";
const char TextInfo30[] PROGMEM = "DHT22 Temperature and Humidity: ";
const char TextInfo31[] PROGMEM = "\nInitializing SD card...";
const char TextInfo32[] PROGMEM = "initialization failed. Things to check:";
const char TextInfo33[] PROGMEM = "* is a card inserted?";
const char TextInfo34[] PROGMEM = "* is your wiring correct?";
const char TextInfo35[] PROGMEM = "* did you change the chipSelect pin to match your shield or module?";
const char TextInfo36[] PROGMEM = "Wiring is correct and a card is present.";
const char TextInfo37[] PROGMEM = "Card type:        ";
const char TextInfo38[] PROGMEM = "SD1";
const char TextInfo39[] PROGMEM = "SD2";
const char TextInfo40[] PROGMEM = "SDHC";
const char TextInfo41[] PROGMEM = "Unknown";
const char TextInfo42[] PROGMEM = "Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card";
const char TextInfo43[] PROGMEM = "Clusters:          ";
const char TextInfo44[] PROGMEM = "Blocks x Cluster:  ";
const char TextInfo45[] PROGMEM = "Total Blocks:      ";
const char TextInfo46[] PROGMEM = "Volume type is:    FAT";
const char TextInfo47[] PROGMEM = "Volume size (Kb):  ";
const char TextInfo48[] PROGMEM = "Volume size (Mb):  ";
const char TextInfo49[] PROGMEM = "Volume size (Gb):  ";
const char TextInfo50[] PROGMEM = "\nFiles found on the card (name, date and size in bytes): ";
const char TextInfo51[] PROGMEM = "Wheater Station";
const char TextInfo52[] PROGMEM = "Version 1.1.8.18";
const char TextInfo53[] PROGMEM = "August 2018";
const char TextInfo54[] PROGMEM = "Adrian Freisinger";
const char TextInfo55[] PROGMEM = "afreisinger@gmail.com";
const char TextInfo56[] PROGMEM = "Everyone is permitted to copy and distribute verbatim copies of this license document, but changing it is not allowed.\n";
const char TextInfo57[] PROGMEM = "                ";


const char* const TextItemPointers[] PROGMEM = {
	TextInfo0,TextInfo1,TextInfo2,TextInfo3,TextInfo4,TextInfo5,TextInfo6,TextInfo7,TextInfo8,TextInfo9,
	TextInfo10,TextInfo11,TextInfo12,TextInfo13,TextInfo14,TextInfo15,TextInfo16,TextInfo17,TextInfo18,TextInfo19,
	TextInfo20,TextInfo21,TextInfo22,TextInfo23,TextInfo24,TextInfo25,TextInfo26,TextInfo27,TextInfo28,TextInfo29,
	TextInfo30,TextInfo31,TextInfo32,TextInfo33,TextInfo34,TextInfo35,TextInfo36,TextInfo37,TextInfo38,TextInfo39,
	TextInfo40,TextInfo41,TextInfo42,TextInfo43,TextInfo44,TextInfo45,TextInfo46,TextInfo47,TextInfo48,TextInfo49,
	TextInfo50,TextInfo51,TextInfo52,TextInfo53,TextInfo54,TextInfo55,TextInfo56, TextInfo57
};

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




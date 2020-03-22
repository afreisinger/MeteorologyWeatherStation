// 
/*

The communication between the microcontroller and the SD card uses SPI, 
which takes place on digital pins 11, 12, and 13 (on most Arduino boards) or 50, 51, and 52 (Arduino Mega). 
Additionally, another pin must be used to select the SD card. This can be the hardware SS pin - pin 10 (on most Arduino boards) or pin 53 (on the Mega) - 
or another pin specified in the call to SD.begin(). Note that even if you don't use the hardware SS pin, it must be left as an output or the SD library won't work.
*/
// 

#include <SD.h>
#include <SdFat.h>

#include <SPI.h>

#include "sd_subrutines.h"
#include "flashrom_subrutines.h"
#include "output_subrutines.h"
#include "flashrom_msg.h"




//#define INITIALIZED_SD_MESSAGE 31
//#include "flashrom_msg.h"
Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;

//DATALOGGER

//File dataFile;   // the logging file
char filename[] = "Temp000.csv";
float tempInCelcius;
float tempInFarenheit;
unsigned long time = 0;
int samplingTime = 10;  //this variable is interval(in Seconds) at which you want to log the data to SD card.
int duration = 15;     //this variable is duration(in Minutes) which is the total time for which you want to log data.





byte mac[6], ip[4], mqqt[4], DNS[4], gateway[4], subnet[4];



void LogToSDCard(String var, File file)
{
	if (file) { file.print(var);  file.flush();
	}
	else Serial.println(GetTextFromFlashMemory(SD_FAILED_MESSAGE));		
}

	




bool InitializingSDCard(uint8_t chipselect)

{
	Serial.print(GetTextFromFlashMemory(SD_INITIALIZED_MESSAGE));
	

	// we'll use the initialization code from the utility libraries
	// since we're just testing if the card is working!
	
	
	if (!card.init(SPI_HALF_SPEED, chipselect)) {
	//if (!SD.begin(SPI_HALF_SPEED, SDCARD_CS)) {
		Serial.print("");
		Serial.println(GetTextFromFlashMemory(SD_FAILED_MESSAGE));
		Serial.println(GetTextFromFlashMemory(SD_INSERTED_MESSAGE));
		Serial.println(GetTextFromFlashMemory(SD_WIRING_MESSAGE));
		Serial.println(GetTextFromFlashMemory(SD_CHIPSELECT_MESSAGE));
		while (1);
	}
	else {
		
		Serial.println(GetTextFromFlashMemory(SD_OK_MESSAGE));
		
	}
	// print the type of card
	Serial.println();
	//Serial.print("Card type:         ");
	Serial.print(GetTextFromFlashMemory(SD_TYPE_MESSAGE));
	
	switch (card.type()) {
	case SD_CARD_TYPE_SD1:
		//Serial.println("SD1");
		Serial.println(GetTextFromFlashMemory(SD_TYPE_SD1_MESSAGE));
		break;
	case SD_CARD_TYPE_SD2:
		//Serial.println("SD2");
		Serial.println(GetTextFromFlashMemory(SD_TYPE_SD2_MESSAGE));
		break;
	case SD_CARD_TYPE_SDHC:
		//Serial.println("SDHC");
		Serial.println(GetTextFromFlashMemory(SD_TYPE_SDHC_MESSAGE));
		break;
	default:
		//Serial.println("Unknown");
		Serial.println(GetTextFromFlashMemory(SD_TYPE_UNKNOWN_MESSAGE));
	}

	// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
	if (!volume.init(card)) {
		Serial.println(GetTextFromFlashMemory(SD_ERROR_MESSAGE));
		while (1);
	}

	//Serial.print("Clusters:          ");
	Serial.println();
	Serial.print(GetTextFromFlashMemory(SD_CLUSTERS_MESSAGE));
	Serial.println(volume.clusterCount());
	
	//Serial.print("Blocks x Cluster:  ");
	Serial.print(GetTextFromFlashMemory(SD_BLOCKS_MESSAGE));
	Serial.println(volume.blocksPerCluster());

	//Serial.print("Total Blocks:      ");
	Serial.print(GetTextFromFlashMemory(SD_TOTAL_BLOCKS_MESSAGE));
	Serial.println(volume.blocksPerCluster() * volume.clusterCount());
	Serial.println();

	// print the type and size of the first FAT-type volume
	uint32_t volumesize;
	//Serial.print("Volume type is:    FAT");

	Serial.print(GetTextFromFlashMemory(SD_VOLUME_TYPE));
	Serial.println(volume.fatType(), DEC);

	volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
	volumesize *= volume.clusterCount();       // we'll have a lot of clusters
	volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
	//Serial.print("Volume size (Kb):  ");
	Serial.print(GetTextFromFlashMemory(SD_VOLUME_KB_MESSAGE));
	Serial.println(volumesize);
	//Serial.print("Volume size (Mb):  ");
	Serial.print(GetTextFromFlashMemory(SD_VOLUME_MB_MESSAGE));
	volumesize /= 1024;
	Serial.println(volumesize);
	//Serial.print("Volume size (Gb):  ");
	Serial.print(GetTextFromFlashMemory(SD_VOLUME_GB_MESSAGE));
	Serial.println((float)volumesize / 1024.0);

	//Serial.println("\nFiles found on the card (name, date and size in bytes): ");
	Serial.println(GetTextFromFlashMemory(SD_FORMATTED_MESSAGE));
	root.openRoot(volume);

	// list all files in the card with date and size
	// root.ls(LS_R | LS_DATE | LS_SIZE);
	Serial.println();
	return true;
	
}





void parseBytes(const char* str, char sep, byte* bytes, int maxBytes, int base) {
	for (int i = 0; i < maxBytes; i++) {
		bytes[i] = strtoul(str, NULL, base);  // Convert byte
		str = strchr(str, sep);               // Find next separator
		if (str == NULL || *str == '\0') {
			break;                            // No more separators, exit
		}
		str++;                                // Point to next character after separator
	}
}










/*


void read_sd() {


	if (!SD.begin(SDCARD_CS))
	{
		Serial.println("ERROR - Failed SD card init.");
		return;    // init failed
	}
	Serial.println("SUCCESS - SD card initialized.");
	//lcd.setCursor(0, 0);
	//lcd.print("SD Card OK");
	//MyDisplayPrint(lcd, GetTextFromFlashMemory(SD_OK_MESSAGE), 0, 0);
	delay(1000);


	if (!SD.exists("config.txt"))
	{
		Serial.println("ERROR - File config.txt not found.");
		return;  // can't find index file
	}
	Serial.println("SUCCESS - Found config.txt file.");

	myFile = SD.open("config.txt", FILE_READ);



	if (myFile) {


		String stringOne = "";
		char* labels[] = { "mac","ip","mqqt","dns","gateway","subnet" };
		int index = 0;
		int totalBytes = myFile.size();

		Serial.print("SUCCESS - Open config.txt file, ");
		Serial.print(totalBytes);
		Serial.println("bytes");


		//--Leemos una l�nea de la hoja de texto--------------
		Serial.println("SUCCESS - Reading config.txt.");
		while (myFile.available()) {

			char c = myFile.read();
			stringOne = stringOne + c;

			if (c == 10)//ASCII de nueva de l�nea
			{
				//break;

				String label = labels[index];

				if (stringOne.startsWith(labels[index]))
				{
					stringOne.remove(0, label.length() + 2);
					stringOne.trim();


					Serial.println(label + ": " + stringOne);


					char _stringOne[stringOne.length() + 1];
					stringOne.toCharArray(_stringOne, sizeof(_stringOne));

					switch (index)
					{
					case 0:
						parseBytes(_stringOne, '-', mac, 6, 16);
					case 1:
						parseBytes(_stringOne, '.', ip, 4, 10);
					case 2:
						parseBytes(_stringOne, '.', mqqt, 4, 10);
					case 3:
						parseBytes(_stringOne, '.', DNS, 4, 10);
					case 4:
						parseBytes(_stringOne, '.', gateway, 4, 10);
					case 5:
						parseBytes(_stringOne, '.', subnet, 4, 10);
					}
				}
				stringOne = "";
				index++;
				delay(1000);
			} //end if ASCII de nueva de l�nea
		}
		Serial.println();
		//---------------------------------------------------
		myFile.close(); //cerramos el archivo	
	}
	else {
		Serial.println("ERROR - File config.txt can�t open.");
	}

}*/




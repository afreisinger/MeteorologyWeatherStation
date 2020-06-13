// 
/*

The communication between the microcontroller and the SD card uses SPI, 
which takes place on digital pins 11, 12, and 13 (on most Arduino boards) or 50, 51, and 52 (Arduino Mega). 
Additionally, another pin must be used to select the SD card. This can be the hardware SS pin - pin 10 (on most Arduino boards) or pin 53 (on the Mega) - 
or another pin specified in the call to SD.begin(). Note that even if you don't use the hardware SS pin, it must be left as an output or the SD library won't work.
*/
// 

#include "sdlib.h"
//#include <SdFat.h>



///////////////////////////////////////////////////////////////////////////////
bool sdlib::InitializingSDCard(uint8_t chipSelect) {
	
	Sd2Card card;
		
	if (!card.init(SPI_HALF_SPEED, chipSelect)) {
			Serial.println(GetTextFromFlashMemory(EMPTY_MESSAGE));
			Serial.println(GetTextFromFlashMemory(SD_FAILED_MESSAGE));
			Serial.println(GetTextFromFlashMemory(SD_INSERTED_MESSAGE));
			Serial.println(GetTextFromFlashMemory(SD_WIRING_MESSAGE));
			Serial.println(GetTextFromFlashMemory(SD_CHIPSELECT_MESSAGE));
			while (true);
	}
	else {
		Serial.println(GetTextFromFlashMemory(SD_OK_MESSAGE));
	}
return true;
}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
void sdlib::InfoSDCard() {
	
	Sd2Card card;
	SdVolume volume;
	SdFile root;

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
	return;
}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
void sdlib::LogToSDCard(String var, File file) {

	if (file) { file.print(var);  file.flush();
	}
	else Serial.println(GetTextFromFlashMemory(SD_FAILED_MESSAGE));		
}
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
void sdlib::SpaceOnSDCard() {

	
	Sd2Card card;
	SdVolume volume;
	SdFat::SdFat sd;
	
			
if (!volume.init(card)) {
		Serial.println(GetTextFromFlashMemory(SD_ERROR_MESSAGE));
		while (1);
	}

	//uint32_t freeKBytes = vol.freeClusterCount();

		int32_t freeKBytes = sd.vol()->freeClusterCount();
		
	//uint32_t freeKBytes = volume.freeClusterCount();
			 freeKBytes *= volume.blocksPerCluster()/2;
	uint32_t freeMBytes = freeKBytes/1024;
	
		Serial.println(GetTextFromFlashMemory(SD_FREE_SPACE_MB));
		Serial.print(freeMBytes);

}
///////////////////////////////////////////////////////////////////////////////
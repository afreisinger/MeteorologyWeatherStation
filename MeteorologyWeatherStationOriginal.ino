/////////////////////////////////////////////////////////////////////////////////////
// HARDWARE:
// Arduino Mega - mandatory (also Arduino Uno can be used with many limitations)
// RTC DS3231	- mandatory
// Ethernet Shield W5100 - optional
// LCD 16x2 - optional
// SD Card	- optional
//-----------------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////////////

#include <Time.h>
#include <TimeLib.h>
//Library Time.zip downloaded from http://www.pjrc.com/teensy/td_libs_Time.html
#include <PubSubClient.h>
#include <DigitalIO.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <SPI.h>
#include <SD.h>
//#include "radio_subrutines.h"
#include "globals.h"
#include "output_subrutines.h"
//#include "time_subrutines.h"




/* uncomment next line if ethernet shield is present */
#define ETHERNET_IS_PRESENT

/* uncomment next line if NRF24  is present  */
#define NRF24_IS_PRESENT

/* uncomment next line if SD Card  is present  */
#define SDCARD_IS_PRESENT

/* uncomment next line if LinkSprite 16x2 LCD Keypad Shield is present  */
#define LCD16x2_IS_PRESENT

/* uncomment next line if I2C TFT 1.8" ST7735 is present */
//#define ADAFRUIT_ST7735_IS_PRESENT

/* uncomment next line if DHT22 is present */
#define DHT22_IS_PRESENT


/* uncomment next line if two TMP36 are connected */
//#define TMP36_IS_PRESENT



/* uncomment next line if updating time in the RTC chip is allowed */
#define ENABLE_RTC_UPDATE



#if defined(ETHERNET_IS_PRESENT)
#include <Ethernet.h>
#include <EthernetUdp.h>
/////////////////////////////////////////////////////////////////////////////////////
// Find the nearest server
// http://www.pool.ntp.org/zone/
// or
// http://support.ntp.org/bin/view/Servers/StratumTwoTimeServers

	 IPAddress timeServer(10, 0, 1, 250);				// local time server
	 //IPAddress timeServer(190, 228, 30, 178);			// AR global time server
	//ver No NTP Response. Error getting time cuando las ip no son validas
/////////////////////////////////////////////////////////////////////////////////////
EthernetUDP Udp;
byte MACAddress[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
#define LOCALUDPPORT 8888
#define SDCARD_CS 4										// Chip Select SD Card
#endif
/////////////////////////////////////////////////////////////////////////////////////
#define NTP_CHECK_PERIOD 3600
#define NTP_RETRY_CHECK_PERIOD 60
#define REPORTDS_TIME_PERIOD 1 //60
volatile bool ntp_interrupt_in_progress = false;
volatile bool NTPworking = false;
volatile bool NTPsetstime = false;
volatile bool NTPasked = false;
/////////////////////////////////////////////////////////////////////////////////////
#if !defined(ETHERNET_IS_PRESENT) && defined(LCD16x2_IS_PRESENT)
#define DHCP_RETRY_PERIOD 0
#define LCD_UPDATE_PERIOD 0
#define D4		4	//	4	// LCD
#define D10		10	//	10 // Contraste
#endif

#if defined(ETHERNET_IS_PRESENT) && !defined(LCD16x2_IS_PRESENT)
#define DHCP_RETRY_PERIOD 300
#define LCD_UPDATE_PERIOD 1
#endif

#if !defined(ETHERNET_IS_PRESENT) && !defined(LCD16x2_IS_PRESENT)
#define DHCP_RETRY_PERIOD 60
#define LCD_UPDATE_PERIOD 1
#endif

#if defined(ETHERNET_IS_PRESENT) && defined(LCD16x2_IS_PRESENT)
// En el shield Ethernet pin 10 se utiliza para seleccionar el chip W5100 y el pin 4 para la tarjeta SD
// En el shield LCD pin 10 se utiliza para contraste y pin 4 como D4
//Usamos 30 y 12 para el LCD
#define DHCP_RETRY_PERIOD 300
#define LCD_UPDATE_PERIOD 1
#define D4		30  //  4	// LCD
#define D10		12	//	10 // Contraste
#endif
/////////////////////////////////////////////////////////////////////////////////////
#if defined(LCD16x2_IS_PRESENT)
#include <LiquidCrystal.h>
#include <LCDKeypad.h>
#define RS		8		
#define EN		9				
#define D5		5		
#define D6		6		
#define D7		7
#define CONSTRAST	50
#define COLS		16
#define	ROWS		2
LiquidCrystal lcd (RS, EN, D4, D5, D6, D7);
#endif
/////////////////////////////////////////////////////////////////////////////////////
#if defined(NRF24_IS_PRESENT) && defined(ETHERNET_IS_PRESENT)
//SoftSPI
//Install the digitalIO library
//Open RF24_config.h in a text editor.Uncomment the line #define SOFTSPI
//In your sketch, add #include DigitalIO.h
//Note : Pins are listed as follows and can be modified in the RF24_config.h file
//const uint8_t SOFT_SPI_MISO_PIN = 16;
//const uint8_t SOFT_SPI_MOSI_PIN = 15;
//const uint8_t SOFT_SPI_SCK_PIN = 14;
#include <RF24.h>
//#include <DigitalIO.h>
#define RF24_CE		40					// Chip Enable Activates RX or Tx mode
#define RF24_CS		41					// SPI Chip Select
RF24 radio(RF24_CE, RF24_CS);
#endif
/////////////////////////////////////////////////////////////////////////////////////
#if defined(NRF24_IS_PRESENT) && !defined(ETHERNET_IS_PRESENT)
#include <RF24.h>
#include <DigitalIO.h>
#define RF24_CE		40					// Chip Enable Activates RX or Tx mode
#define RF24_CS		41					// SPI Chip Select
RF24 radio(RF24_CE, RF24_CS);
#endif
/////////////////////////////////////////////////////////////////////////////////////






#if defined(DHT22_IS_PRESENT)
#include <DHT.h>
#define TEMPERATURE_SENSOR_DHT22 1
#define REMOTE_TEMPERATURE_SENSOR_DHT22 5
#define TEMPERATURE_SENSOR_PIN	42
#define TEMPERATURE_SENSOR_TYPE DHT22
DHT dht(TEMPERATURE_SENSOR_PIN, TEMPERATURE_SENSOR_TYPE);       //pin, tipo
#endif
/////////////////////////////////////////////////////////////////////////////////////
#define TIME_SYSTEM_CLOCK              0b0000000000000001
#define TIME_RTC_CLOCK                 0b0000000000000010
#define TIME_FORMAT_SMTP_DATE          0b0000000000000100
#define TIME_FORMAT_SMTP_MSGID         0b0000000000001000
#define TIME_FORMAT_HUMAN              0b0000000000010000
#define TIME_FORMAT_HUMAN_SHORT        0b0000000000100000
#define TIME_FORMAT_TIME_ONLY          0b0000000001000000
#define TIME_FORMAT_TIME_ONLY_WO_COLON 0b0000000010000000
#define TIME_FORMAT_DATE_ONLY          0b0000000100000000
#define DAYLIGHTSAVING_START_MONTH 1 // 
#define DAYLIGHTSAVING_START_HOUR  1 // 2:00:00 -> 3:00:00
#define DAYLIGHTSAVING_STOP_MONTH  1 // 
#define DAYLIGHTSAVING_STOP_HOUR   1 // 3:00:00 -> 2:00:00
#define DAYLIGHTSAVING_TIMESHIFT   0 // Desplazamiento
#define TIME_ZONE_INT -3			 // Time Zone
#define TIME_ZONE_NODST "+0000"
#define TIME_ZONE_DST "+0000"
String TIME_ZONE = TIME_ZONE_NODST;
time_t time_dhcp_retry = 0;
time_t time_last_ntp_check = 0;
time_t time_last_reportDS = 0;
time_t time_last_lcd = 0;
time_t rtc_now = 0;
time_t rtc_epoch = 0;
tmElements_t tm;
/////////////////////////////////////////////////////////////////////////////////////
#define DS3231_ADDRESS 104
int rtc_second;			//00-59;
int rtc_minute;			//00-59;
int rtc_hour;			//1-12 - 00-23;
int rtc_weekday;		//1-7
int rtc_day;			//01-31
int rtc_month;			//01-12
int rtc_year;			//0-99 + 2000;
#define TEMPERATURE_SENSOR_DS3231  0  // RTC - zde muze byt cokoli krome A2 a A3, coz je 16 a 17
#define TEMPERATURE_SENSOR_TMP36_1 A2   // TMP36 inside
#define TEMPERATURE_SENSOR_TMP36_2 A3   // TMP36 outside
#define MAX_TEMPERATURE_EXCEED 111.11   // Celsius, bad sensor if exceed
bool RTCworking = false;
/////////////////////////////////////////////////////////////////////////////////////
#define BAUDRATE 115200
/////////////////////////////////////////////////////////////////////////////////////
#define BUFFERSIZE 85
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
const char TextInfo22[] PROGMEM = "RTC time and temperature: ";
const char TextInfo23[] PROGMEM = "Failed to get the current time from RTC chip.";
const char TextInfo24[] PROGMEM = "No data (temperature sensor error).";
const char TextInfo25[] PROGMEM = "Current time failed to get either the NTP server or the RTC chip.";
const char TextInfo26[] PROGMEM = "TMP36: ";
const char TextInfo27[] PROGMEM = "Unable to compare NTP and RTC time (RTC is not present?)";
const char TextInfo28[] PROGMEM = "Error calling function: function ComposeTimeStamp requires time source (CPU or RTC).";
const char TextInfo29[] PROGMEM = "Error getting time";
const char TextInfo30[] PROGMEM = "DHT22 temperature and humidity: ";

const char* const TextItemPointers[] PROGMEM = {
	TextInfo0,TextInfo1,TextInfo2,TextInfo3,TextInfo4,TextInfo5,TextInfo6,TextInfo7,TextInfo8,TextInfo9,TextInfo10,TextInfo11,TextInfo12,TextInfo13,TextInfo14,TextInfo15,TextInfo16,TextInfo17,TextInfo18,TextInfo19,TextInfo20,TextInfo21,TextInfo22,TextInfo23,TextInfo24,TextInfo25,TextInfo26,TextInfo27,TextInfo28,TextInfo29,TextInfo30
};
#define WELCOME_MESSAGE              0
#define UNINITIALIZED_RTC_MESSAGE    1
#define DHCP_WAIT_MESSAGE            2
#define DHCP_FAIL_MESSAGE            3
#define DHCP_PASS_MESSAGE            4
#define IP_MESSAGE                   5
#define MASK_MESSAGE                 6
#define GATEWAY_MESSAGE              7
#define DNS_MESSAGE                  8
#define MAC_MESSAGE                  9
#define NTP_WAIT_MESSAGE            10
#define NTP_SEND_MESSAGE            11
#define NTP_RESPONSES_MESSAGE       12
#define NTP_NO_RESPONSE_MESSAGE     13
#define NTP_ERROR_MESSAGE           14
#define RTC_TIME_MESSAGE            15
#define RTC_OK_MESSAGE              16
#define RTC_NEED_SYNC_MESSAGE       17
#define SECONDS_MESSAGE             18
#define RTC_SYNC_MESSAGE            19
#define RTC_TF_MESSAGE              20
#define RTC_DF_MESSAGE              21
#define INFO_MESSAGE                22
#define RTC_ERROR_MESSAGE           23
#define TEMPERATURE_ERRROR_MESSAGE  24
#define TIME_ERRROR_MESSAGE         25
#define SENSORS_MESSAGE             26
#define RTC_NOT_PRESENT             27
#define TIMESOURCE_ERRROR_MESSAGE   28
#define ERROR_MESSAGE				29
#define SENSORS_MESSAGE_2           30


Sd2Card card;
SdVolume volume;
SdFile root;
File myFile;

byte mac[6], ip[4], mqqt[4] ,DNS[4], gateway[4], subnet[4];

long lastMsg = 0;
char msg[50];
int value = 0;
char temp[50];
char hum[50];


//EthernetClient ethClient;
//PubSubClient client(server, 1883, callback, ethClient);

EthernetClient ethClient;
PubSubClient mqtt(ethClient);


File webFile, configFile;               // El fichero de la pagina web en la SD card


/************ Progess Bar ************/

#define lenght_bar 16.0			//Longitud de de la barra, total 16 caracteres

double percent = 100.0;
unsigned char b;
unsigned int piece;

/************ Progess Bar ************/





//RTC_DS3231 rtc;


struct package
{
	float temperature = 0.0;
	float humidity = 0.0;
};

typedef struct package Package;

Package data;



void setup() {
	
	// disable SD SPI
	//pinMode(4, OUTPUT);
	//digitalWrite(4, HIGH);

	// Enable SD SPI
	//pinMode(4, OUTPUT);
	//digitalWrite(4, LOW);

	// disable w5100 SPI
	//pinMode(10, OUTPUT);
	//digitalWrite(10, HIGH);

	
	Serial.begin(BAUDRATE, SERIAL_8N1);

	//sd_setup();
	//rtc_setup();
	//radio_setup(radio);
	
	
	/*
	byte address[][6] = { "1Node","2Node","3Node" };
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
	*/
	

	Wire.begin();


#if defined(LCD16x2_IS_PRESENT)
	lcd.begin(COLS,ROWS);
	lcd.clear();
	lcd.createChar(0, BLOCK_1x8);	// 1/5 full block
	lcd.createChar(1, BLOCK_2x8);	// 2/5 full block
	lcd.createChar(2, BLOCK_3x8);	// 3/5 full block
	lcd.createChar(3, BLOCK_4x8);	// 4/5 full block
	lcd.createChar(4, BLOCK_5x8);	// full block
	lcd.createChar(5, GRADO);		// º
	#if defined(ETHERNET_IS_PRESENT)		//wire hacking
		pinMode(D10, OUTPUT);
		analogWrite(D10, CONSTRAST);
	#endif
#endif
	
	//read_sd();
	//ethernet_setup();
	//mqqt_setup();
	//reconnect();

	
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
#define LCDbacklightPin 3
	pinMode(LCDbacklightPin, OUTPUT);
	//digitalWrite(LCDbacklightPin, HIGH);
	analogWrite(LCDbacklightPin, 255);
	delay(5000);

	tft.initR(INITR_BLACKTAB);
	//tft.initR(INITR_GREENTAB);
	//tft.initR(INITR_REDTAB);

	tft.setRotation(1);
	tft.setTextWrap(true);
	tft.fillScreen(ST7735_BLACK);
	tft.setTextColor(ST7735_RED, ST7735_YELLOW);
	tft.setTextSize(3);
	tft.setCursor(8, 50);
	tft.println("RTC test");
	tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
	tft.setTextSize(1);
	tft.setCursor(0, 0);
	tft.println(GetTextFromFlashMemory(WELCOME_MESSAGE));
#endif
	
#if defined(TMP36_IS_PRESENT)
	pinMode(TEMPERATURE_SENSOR_TMP36_1, INPUT);
	pinMode(TEMPERATURE_SENSOR_TMP36_2, INPUT);
#endif
#if defined(DHT22_IS_PRESENT)
	dht.begin();
	//delay(1000);
#endif
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
	tft.setTextSize(2);
	tft.setCursor(0, 80);
#endif
#if defined(ETHERNET_IS_PRESENT) || defined(DHT22_IS_PRESENT)
	//pinMode(SDCARDPin, OUTPUT);
	//digitalWrite(SDCARDPin, HIGH);
	// delay for terminal
	for (byte index = 5; index > 0; index--) {
		delay(1000);
		MyPrint(String(index) + " ");
	};
	MyPrintLn("");
#else
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.fillScreen(ST7735_BLACK);
#endif
#endif
	
	
	MySerialPrint(GetTextFromFlashMemory(WELCOME_MESSAGE) + "\r\n");
	time_t rtc_uninitialized_time = 0;
	if (read_rtc(rtc_uninitialized_time)) {setTime(rtc_uninitialized_time);};
	MySerialPrint(GetTextFromFlashMemory(UNINITIALIZED_RTC_MESSAGE) + "\r\n");
	MySerialPrint(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, rtc_uninitialized_time));
	MySerialPrint(" ");
	MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, false) + "\r\n");
}




void loop() {
	
	
	//rx_radio();

	/*
	if (radio.available())							//check when received data available
	{
		radio.read(&data, sizeof(data));
		//temperature_display();
		//date_serial;
		//String temperature_dht = (String)data.temperature;
		//String humedity_dht = (String)data.humidity;
		Serial.print("Remoto: ");
		Serial.print(data.temperature);
		Serial.print(" ");
		Serial.print(data.humidity);
		Serial.println(" ");
		delay(1000);
	}

	*/



	
	
	

	if (checkdhcp()) {
		while (true) {
			if (checkntp()) {
				time_last_ntp_check = now();
			}
			else {
				time_last_ntp_check = now() - NTP_CHECK_PERIOD + NTP_RETRY_CHECK_PERIOD;
			};
			
			while (time_last_ntp_check + NTP_CHECK_PERIOD > now()) {
					
				ShowTime(false);
				MaintainTimeSources(false); //if NTP server was asked by system time updater, show results			
			};
		};
	}
	else 
	{
		if (rtc_epoch == 0) {
			if (read_rtc(rtc_epoch)) {
				setTime(rtc_epoch);
			};
		};
		time_dhcp_retry = now();
		bool FirstTime = true;
		while (time_dhcp_retry + DHCP_RETRY_PERIOD >= now()) {
			ShowTime(FirstTime);
			
			if (FirstTime) {
				FirstTime = false;
			};
		};
	};


	



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//if (!mqtt.connected()) { reconnect(); } mqtt.loop();
//	mqtt.loop();
	//mqtt_display();
	// Obtener fecha actual y mostrar por Serial
	//DateTime now = rtc.now();

	//date_display(now);   
	//date_serial(now);
	
	//delay(1000);



	

	
	
	/*
	String temperature_dht = (String)data.temperature;
	temperature_dht.toCharArray(temp, temperature_dht.length() + 1);
	
	String humedity_dht = (String)data.humidity;
	humedity_dht.toCharArray(hum, humedity_dht.length() + 1);
	
	
	int ndata = 6;
	
	String payload;
	payload += ndata;
	payload += " " + temperature_dht;
	payload += " " + humedity_dht;
	payload += " " + temperature_dht;
	payload += " " + humedity_dht;
	payload += " " + temperature_dht;
	payload += " " + humedity_dht;
	payload += " ";
	payload.toCharArray(msg, payload.length() + 1);
	*/
	
	/*
	int leng_str = payload.length();
	byte payload[leng_str];
	int LengthFrameAPI = 18 + sizeof(payload);
	int LengthPayload = sizeof(payload);
	*/
	
	
	
		
	/*
	unsigned long startTimer = millis(); //start timer, we will wait 200ms 

	bool timeout = false;

	while (!mqtt.connected() && !timeout) { //run while no receive data and not timed out
		int j = 1;

		if (millis() - startTimer > 60000) timeout = true; //timed out
		reconnect(); // cada 5 segundos
		j++;
		Serial.println(j);
	}
	
	*/
	

		/*
		long timelapse = millis();
		if (timelapse - lastMsg > 2000) {
			lastMsg = timelapse;
			++value;
			//Serial.print("Publish message: ");
			//Serial.println(msg);
			mqtt.publish("outTopic", msg);

			mqtt.publish("/sensors/iolcity/weather/temperature", temp);
			mqtt.publish("/sensors/iolcity/weather/humidity", hum);
			String json =
				"{\"data\":{"
				"\"humidity\": \"" + String(hum) + "\","
				"\"temperature\": \"" + String(temp) + "\"}"
				"}";


			// Convert JSON string to character array
			char jsonChar[100];
			json.toCharArray(jsonChar, json.length() + 1);

			// Publish JSON character array to MQTT topic
			mqtt.publish("/sensors/iolcity/weather/json", jsonChar);
			
			//Serial.println(" ");
			//Serial.println("Data");
			//Serial.println(json);

		}*/

	}

	void rx_radio() {

		if (radio.available())							//check when received data available
		{
			radio.read(&data, sizeof(data));
			//temperature_display();
			//date_serial;
			//String temperature_dht = (String)data.temperature;
			//String humedity_dht = (String)data.humidity;
			Serial.print("Remoto: ");
			Serial.print(data.temperature);
			Serial.print(" ");
			Serial.print(data.humidity);
			Serial.println(" ");
			delay(1000);
		}


}


void sd_setup() {

	Serial.print("\nInitializing SD card...");

	// we'll use the initialization code from the utility libraries
	// since we're just testing if the card is working!
	if (!card.init(SPI_HALF_SPEED, SDCARD_CS)) {
		Serial.println("initialization failed. Things to check:");
		Serial.println("* is a card inserted?");
		Serial.println("* is your wiring correct?");
		Serial.println("* did you change the chipSelect pin to match your shield or module?");
		while (1);
	}
	else {
		Serial.println("Wiring is correct and a card is present.");
	}
	// print the type of card
	Serial.println();
	Serial.print("Card type:         ");
	switch (card.type()) {
	case SD_CARD_TYPE_SD1:
		Serial.println("SD1");
		break;
	case SD_CARD_TYPE_SD2:
		Serial.println("SD2");
		break;
	case SD_CARD_TYPE_SDHC:
		Serial.println("SDHC");
		break;
	default:
		Serial.println("Unknown");
	}

	// Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
	if (!volume.init(card)) {
		Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
		while (1);
	}

	Serial.print("Clusters:          ");
	Serial.println(volume.clusterCount());
	Serial.print("Blocks x Cluster:  ");
	Serial.println(volume.blocksPerCluster());

	Serial.print("Total Blocks:      ");
	Serial.println(volume.blocksPerCluster() * volume.clusterCount());
	Serial.println();

	// print the type and size of the first FAT-type volume
	uint32_t volumesize;
	Serial.print("Volume type is:    FAT");
	Serial.println(volume.fatType(), DEC);

	volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
	volumesize *= volume.clusterCount();       // we'll have a lot of clusters
	volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
	Serial.print("Volume size (Kb):  ");
	Serial.println(volumesize);
	Serial.print("Volume size (Mb):  ");
	volumesize /= 1024;
	Serial.println(volumesize);
	Serial.print("Volume size (Gb):  ");
	Serial.println((float)volumesize / 1024.0);

	Serial.println("\nFiles found on the card (name, date and size in bytes): ");
	root.openRoot(volume);

	// list all files in the card with date and size
	//root.ls(LS_R | LS_DATE | LS_SIZE);
	//(Serial.println();
	
}

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
	
		myFile = SD.open("config.txt",FILE_READ);
		

	
	if (myFile) {
		

		String stringOne = "";
		char* labels[] = { "mac","ip","mqqt","dns","gateway","subnet" };
		int index = 0;
		int totalBytes = myFile.size();
		
		Serial.print("SUCCESS - Open config.txt file, ");
		Serial.print(totalBytes);
		Serial.println("bytes");

		
		//--Leemos una línea de la hoja de texto--------------
		Serial.println("SUCCESS - Reading config.txt.");
		while (myFile.available()) {

			char c = myFile.read();
			stringOne = stringOne + c;
			
			if (c == 10)//ASCII de nueva de línea
				{
				//break;
			
					String label = labels[index];
				
						if (stringOne.startsWith(labels[index])) 
						{
							stringOne.remove(0, label.length()+2);
							stringOne.trim();
							
						
							Serial.println(label + ": " + stringOne);
						
					
							char _stringOne[stringOne.length() + 1];
							stringOne.toCharArray(_stringOne,sizeof(_stringOne));
					
								switch (index) 
											{
											case 0:
												parseBytes(_stringOne, '-', mac,6 ,16);
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
			} //end if ASCII de nueva de línea
		}
		Serial.println();
		//---------------------------------------------------
		myFile.close(); //cerramos el archivo	
	}
	else {
		Serial.println("ERROR - File config.txt can´t open.");
	}

	}

/*
void rtc_setup() {

	if (!rtc.begin()) {
		Serial.println(F("Couldn't find RTC"));
		while (1);
	}

	// Si se ha perdido la corriente, fijar fecha y hora
	if (rtc.lostPower()) {
		// Fijar a fecha y hora de compilacion
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

		// Fijar a fecha y hora específica. En el ejemplo, 21 de Enero de 2016 a las 03:00:00
		 //rtc.adjust(DateTime(2016, 1, 21, 3, 0, 0));
		  // rtc.adjust(DateTime(2018, 5, 9, 9, 40, 0));
	}
}
*/

void ethernet_setup() {

	Serial.println("SUCCESS - W5100 card initialized.");

	Ethernet.begin(mac, ip, DNS, gateway, subnet);

	Serial.print("ip: ");  Serial.println(Ethernet.localIP());
	Serial.print("dns: "); Serial.println(Ethernet.dnsServerIP());
	Serial.print("gateway: "); Serial.println(Ethernet.gatewayIP());
	Serial.print("subnet: "); Serial.println(Ethernet.subnetMask());
	Serial.println();
}

void mqqt_setup() {
	mqtt.setServer(mqqt, 1883);
	mqtt.setCallback(callback);
}

void temperature_display() {
	
	lcd.setCursor(0, 1);
	lcd.print(data.temperature);
	lcd.write(5);									//Muestra primer caracter º
	lcd.print(" ");
	lcd.print(data.humidity);
	lcd.print("%");
}

void date_display(DateTime date)
{
	lcd.setCursor(0, 0);
	lcd.print(date.day(), DEC);
	lcd.print('/');
	lcd.print(date.month(), DEC);
	lcd.print('/');
	lcd.print(date.year()-2000, DEC);
	lcd.print(' ');
	printDigits(date.hour());
	lcd.print(':');
	printDigits(date.minute());
	lcd.print(':');
	printDigits(date.second());

}

void printDigits(int digits)    // Modifico esta funcion para que a la vez que imprime 
{
	
						   //lcd.print(":");         
	if (digits < 10)
	{
		lcd.print('0');
		
	}
	
		lcd.print(digits);
}

void date_serial(DateTime date)
{
	Serial.print(daysOfTheWeek[date.dayOfTheWeek()]);
	Serial.print(" ");
	Serial.print(date.day(), DEC);
	Serial.print('/');
	Serial.print(date.month(), DEC);
	Serial.print('/');
	Serial.print(date.year(), DEC);
	Serial.print(" ");
	printDigitsSerial(date.hour());
	//Serial.print(date.hour(), DEC);
	Serial.print(':');
	printDigitsSerial(date.minute());
	//Serial.print(date.minute(), DEC);
	Serial.print(':');
	printDigitsSerial(date.second());
	Serial.print(" ");
	//Serial.print(date.second(), DEC);
	Serial.print(data.temperature);
	Serial.print(" ");
	Serial.print(data.humidity);
	Serial.print(" ");
	Serial.println();
}

void printDigitsSerial(int digits)    // Modifico esta funcion para que a la vez que imprime 
{
	    // via serie tambien lo haga via LCD
						   //lcd.print(":");         
	if (digits < 10)
	{
		Serial.print('0');
		
	}
	Serial.print(digits, DEC);
	
}

void SwitchRfEth()
{	//added it before and after calling nRF;

	//Switch ethernet's CS pin state
	digitalWrite(10, !digitalRead(10));
	//Switch radio's CS pin state
	digitalWrite(53, !digitalRead(53));
}

void callback(char* topic, byte* payload, unsigned int length) {

	Serial.print("Payload: ");
	Serial.write(payload, length);
	Serial.println();

}

void reconnect() {
	// Loop until we're reconnected
	
	unsigned long startTimer = millis(); //start timer, we will wait 30seg 
	bool timeout = false;
	int lapse = 30000;
	
	int j = 0;
	value = 1024/lenght_bar;
	while (!mqtt.connected() && !timeout) {
		j++;
		Serial.print("j");
		Serial.println(j);

		if (millis() - startTimer > lapse) timeout = true; //timed out
		
		
		//Serial.println(timeout);
		Serial.println(millis() - startTimer);
		//Serial.println(j);
		
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (mqtt.connect("arduinoClient")) {
			Serial.println("connected");
			// Once connected, publish an announcement...
			mqtt.publish("outTopic", "hello world");
			// ... and resubscribe
			mqtt.subscribe("inTopic");
		}
		else {
			Serial.print("failed, rc=");
			//Serial.print(mqtt.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			Serial.print("Value:");
			Serial.println(value);
			
			mqtt_display();
			value = value + (1024 / lenght_bar);
			delay(lapse/(3*lenght_bar));
			
		}
	}
}

void mqtt_display() {


	lcd.setCursor(0, 0);
	lcd.print("Conectando");
	
	lcd.setCursor(0, 1);

	percent = value / 1024.0 * 100;
	Serial.print("percent:");
	Serial.println(percent);

	double a = lenght_bar / 100 * percent;

	// drawing black rectangles on LCD

	if (a >= 1) {
		for (int i = 1; i < a; i++) {
			lcd.write(4);
			b = i;
		}
		a = a - b;
	}
	piece = a * 5;
	// drawing charater's colums
	switch (piece) {

	case 0:
		break;
	case 1:
		lcd.print((char)0);
		//lcd.write(0);
		break;
	case 2:
		lcd.write(1);
		break;
	case 3:
		lcd.write(2);
		break;
	case 4:
		lcd.write(3);
		break;
	}

	//clearing line
	for (int i = 0; i < (lenght_bar - b); i++) {
		lcd.print(" ");
	}
	;
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

void printIPAddress()
{
	Serial.print("My IP address: ");
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		// print the value of each byte of the IP address:
		Serial.print(Ethernet.localIP()[thisByte], DEC);
		Serial.print(".");
	}

}

/////////////////////////////////////////////////////////////////////////////////////
/*-------- FLASH ROM subroutine ----------*/
/////////////////////////////////////////////////////////////////////////////////////
String GetTextFromFlashMemory(int ItemIndex) //function to return string by index
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
/////////////////////////////////////////////////////////////////////////////////////
/*-------- Time subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
time_t EvaluateDaylightSavingTime(time_t result) {
	if (
		(
		(                                                                  // plati od
			(month(result) == DAYLIGHTSAVING_START_MONTH)                      // breznove
			&& (month(nextSunday(result)) == (DAYLIGHTSAVING_START_MONTH + 1))  // posledni nedele
			&&
			(
			(weekday(result) != 1)
				||
				(hour(result) >= DAYLIGHTSAVING_START_HOUR)                      // od dvou hodin rano
				)
			)
			|| (month(result) > DAYLIGHTSAVING_START_MONTH)                 // a v nasledujich mesicich
			)
		&&
		(
		(                                                                  // plati do
			(month(result) == DAYLIGHTSAVING_STOP_MONTH)                       // rijnove
			&& (month(nextSunday(result)) == DAYLIGHTSAVING_STOP_MONTH)       // posledni nedele
			)
			||
			(
			(month(result) == DAYLIGHTSAVING_STOP_MONTH)
				&& (month(nextSunday(result)) == (DAYLIGHTSAVING_STOP_MONTH + 1))
				&& (weekday(result) == 1)
				&& (hour(result) < DAYLIGHTSAVING_STOP_HOUR)
				)
			|| (month(result) < DAYLIGHTSAVING_STOP_MONTH)                  // a v predchozich mesicich
			)
		) {
		result = result + DAYLIGHTSAVING_TIMESHIFT * SECS_PER_HOUR;
		TIME_ZONE = TIME_ZONE_DST;
	}
	else {
		TIME_ZONE = TIME_ZONE_NODST;
	};
	return result;
};
byte decToBcd(byte val) {/////////////////////////////////////////////////////////////////////////////////////
	return ((val / 10 * 16) + (val % 10));
};
byte bcdToDec(byte val)
{
	return ((val / 16 * 10) + (val % 16));
};
bool set_rtc_datum() {/////////////////////////////////////////////////////////////////////////////////////
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(3);//set register to day
	Wire.write(decToBcd(rtc_weekday));
	Wire.write(decToBcd(rtc_day));
	Wire.write(decToBcd(rtc_month));
	Wire.write(decToBcd(rtc_year - 2000));
	if (Wire.endTransmission() == 0) {
		return true;
	}
	else {
		return false;
	};
};
bool set_rtc_time() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(0);//set register to time
	Wire.write(decToBcd(rtc_second));
	Wire.write(decToBcd(rtc_minute));
	Wire.write(decToBcd(rtc_hour));
	if (Wire.endTransmission() == 0) {
		return true;
	}
	else {
		return false;
	};
};
bool get_rtc_datum() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(3);//set register to day
	Wire.endTransmission();
	if (Wire.requestFrom(DS3231_ADDRESS, 4) == 4) { //get 4 bytes(day,date,month,year);
		rtc_weekday = bcdToDec(Wire.read());
		rtc_day = bcdToDec(Wire.read());
		rtc_month = bcdToDec(Wire.read());
		rtc_year = bcdToDec(Wire.read()) + 2000;
		RTCworking = true;
		return true;
	}
	else {
		RTCworking = false;
		return false;
	};
};
bool get_rtc_time() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(0);//set register to time
	Wire.endTransmission();
	if (Wire.requestFrom(DS3231_ADDRESS, 3) == 3) { //get 3 bytes (seconds,minutes,hours);
		rtc_second = bcdToDec(Wire.read() & 0x7f);
		rtc_minute = bcdToDec(Wire.read());
		rtc_hour = bcdToDec(Wire.read() & 0x3f);
		return true;
	}
	else {
		return false;
	};
};
float get_rtc_temperature() {
	Wire.beginTransmission(DS3231_ADDRESS);
	Wire.write(17);//set register to DS3132 internal temperature sensor
	Wire.endTransmission();
	if (Wire.requestFrom(DS3231_ADDRESS, 2) == 2) {
		float ttc = (float)(int)Wire.read();
		byte portion = Wire.read();
		if (portion == 0b01000000) ttc += 0.25;
		if (portion == 0b10000000) ttc += 0.5;
		if (portion == 0b11000000) ttc += 0.75;
		return ttc;
	}
	else {
		return MAX_TEMPERATURE_EXCEED;
	};
};

#if defined(DHT22_IS_PRESENT)
float get_dht_temperature() {
	float  t = dht.readTemperature();
	return t;
}
float get_dht_humedity() {
	float  h = dht.readHumidity();
	return h;
}
#endif

bool read_rtc(time_t& cas) {
	if (get_rtc_datum()) {
		get_rtc_time();
		tm.Year = rtc_year - 1970;
		tm.Month = rtc_month;
		tm.Day = rtc_day;
		tm.Hour = rtc_hour;
		tm.Minute = rtc_minute;
		tm.Second = rtc_second;
		cas = makeTime(tm);
		return true;
	}
	else {
		return false;
	};
};
/////////////////////////////////////////////////////////////////////////////////////
/*-------- PrintTime subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
String ComposeZerosLeadedNumber(unsigned long int MyNumber, byte NumberOfCharacters) {
	String TempString = "";
	for (byte index = 1; index <= NumberOfCharacters; index++) {
		TempString = "0" + TempString;
	};
	TempString = TempString + String(MyNumber);
	int ifrom = TempString.length() - NumberOfCharacters;
	int ito = TempString.length();
	return TempString.substring(ifrom, ito);
};
String ComposeTimeStamp(unsigned int details, time_t datum) {
	String strday;
	String strmonth;
	String stryear;
	String strhour;
	String strminute;
	String strsecond;
	String strweekday;
	int myvalue;
	if (
		((details & TIME_RTC_CLOCK) != TIME_RTC_CLOCK)
		&&
		((details & TIME_SYSTEM_CLOCK) != TIME_SYSTEM_CLOCK)
		) {
		strday = GetTextFromFlashMemory(TIMESOURCE_ERRROR_MESSAGE);
		return strday;
	};
	if ((details & TIME_RTC_CLOCK) == TIME_RTC_CLOCK) {
		if (datum <= 0) {
			if (!read_rtc(datum)) {
				if (NTPworking) {
					strday = GetTextFromFlashMemory(RTC_ERROR_MESSAGE);
				}
				else {
					strday = GetTextFromFlashMemory(TIME_ERRROR_MESSAGE);
				};
				return strday;
			};
		};
	};
	if ((details & TIME_SYSTEM_CLOCK) == TIME_SYSTEM_CLOCK) {
		if (datum <= 0) {
			datum = now();
		};
	};
	datum = EvaluateDaylightSavingTime(datum);
	strday = ComposeZerosLeadedNumber(day(datum), 2);
	myvalue = month(datum);
	if ((details & TIME_FORMAT_SMTP_DATE) == TIME_FORMAT_SMTP_DATE) {
		strmonth = monthShortStr(myvalue);
	}
	else {
		strmonth = ComposeZerosLeadedNumber(myvalue, 2);
	};
	stryear = String(year(datum));
	strhour = ComposeZerosLeadedNumber(hour(datum), 2);
	strminute = ComposeZerosLeadedNumber(minute(datum), 2);
	strsecond = ComposeZerosLeadedNumber(second(datum), 2);
	int tweekday;
	if (details & TIME_RTC_CLOCK) {
		tweekday = rtc_weekday;
	}
	else {
		tweekday = weekday(datum);
	};
	if ((details & TIME_FORMAT_HUMAN) == TIME_FORMAT_HUMAN) {
		strweekday = dayStr(tweekday);
	}
	else {
		strweekday = dayShortStr(tweekday);
	};
	if ((details & TIME_FORMAT_HUMAN) == TIME_FORMAT_HUMAN) {
		return String(strday + "." + strmonth + "." + stryear + " " + strhour + ":" + strminute + ":" + strsecond + " " + strweekday);
	}
	else {
		if ((details & TIME_FORMAT_HUMAN_SHORT) == TIME_FORMAT_HUMAN_SHORT) {
			return String(strday + "." + strmonth + "." + stryear + " " + strhour + ":" + strminute + ":" + strsecond + " ");//20 characters for LCD
		}
		else {
			if ((details & TIME_FORMAT_TIME_ONLY) == TIME_FORMAT_TIME_ONLY) {
				return String(strhour + ":" + strminute + ":" + strsecond);
			}
			else {
				if ((details & TIME_FORMAT_TIME_ONLY_WO_COLON) == TIME_FORMAT_TIME_ONLY_WO_COLON) {
					return String(strhour + " " + strminute + " " + strsecond);
				}
				else {
					if ((details & TIME_FORMAT_DATE_ONLY) == TIME_FORMAT_DATE_ONLY) {
						return String(strday + "." + strmonth + "." + stryear);
					}
					else {
						if ((details & TIME_FORMAT_SMTP_MSGID) == TIME_FORMAT_SMTP_MSGID) {
							return String(stryear + strmonth + strday + strhour + strminute + strsecond + ".") + String(millis());
						}
						else {
							return String(strweekday + ", " + strday + " " + strmonth + " " + stryear + " " + strhour + ":" + strminute + ":" + strsecond + " " + TIME_ZONE);
						};
					};
				};
			};
		};
	};
};

/*
float ComposeTemperatureValue(byte sensor) {
	String TempStr = "";
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float TemperatureC1, TemperatureC2;
	if (sensor == TEMPERATURE_SENSOR_DS3231) {
		TemperatureC1 = get_rtc_temperature();
	}
	else {
		do
		{
			TemperatureC1 = (((analogRead(sensor) * (5000.0 / 1024)) - 750) / 10) + 25;
			delay(10);
			TemperatureC2 = (((analogRead(sensor) * (5000.0 / 1024)) - 750) / 10) + 25;
		} while (TemperatureC1 != TemperatureC2);
	};
	return TemperatureC1;
};

*/



float ComposeTemperatureValue(byte sensor) {
	String TempStr = "";
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float TemperatureC1, TemperatureC2;
	if (sensor == TEMPERATURE_SENSOR_DS3231) {
		TemperatureC1 = get_rtc_temperature();
	}
	
#if defined(DHT22_IS_PRESENT)
	else if (sensor == TEMPERATURE_SENSOR_DHT22)
	{
		TemperatureC1 = get_dht_temperature();
	}
#endif
	else if (sensor == REMOTE_TEMPERATURE_SENSOR_DHT22)
	{
		TemperatureC1 = data.temperature;
	}

	else	
	{
		TemperatureC1 = (((analogRead(sensor) * (5000.0 / 1024)) - 750) / 10) + 25;
	}
	
	
	return TemperatureC1;
};











float ComposeHumedityValue(byte sensor) {
	float HumedityH1;
#if defined(DHT22_IS_PRESENT)
		if (sensor == TEMPERATURE_SENSOR_DHT22)
	{
			HumedityH1 = get_dht_humedity();
	}
#endif
		return HumedityH1;
}




////////////////////////////////////////////////////////////////////////////////////
String ComposeHumedityString(byte sensor, bool displaytype) {
	String HumStr = "";
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float HumedityH = ComposeHumedityValue(sensor);
	if (HumedityH < 100) {
		char buffer[16];
	HumStr = dtostrf(HumedityH, 5, 2, buffer);
		if (displaytype) {
			HumStr = HumStr + "%"; //oC for Asian Character Set
		}
		else {
			HumStr = HumStr + "%"; //oC for Western Character Set
		};
	}
	else {
		//error message
		HumStr = GetTextFromFlashMemory(TEMPERATURE_ERRROR_MESSAGE);
	};
	return HumStr;
};
////////////////////////////////////////////////////////////////////////////////////









////////////////////////////////////////////////////////////////////////////////////
String ComposeTemperatureString(byte sensor, bool displaytype) {
	String TempStr = "";
	// Convert Analog value to temperature
	// 10 mV na 1 stupen Celsia/Kevina
	float TemperatureC = ComposeTemperatureValue(sensor);
	if (TemperatureC < MAX_TEMPERATURE_EXCEED) {
		char buffer[16];
		TempStr = dtostrf(TemperatureC, 5, 2, buffer);
		if (displaytype) {
			TempStr = TempStr + "\xDF\x43"; //oC for Asian Character Set
		}
		else {
			TempStr = TempStr + "\xF8\x43"; //oC for Western Character Set
		};
	}
	else {
		//error message
		TempStr = GetTextFromFlashMemory(TEMPERATURE_ERRROR_MESSAGE);
	};
	return TempStr;
};
////////////////////////////////////////////////////////////////////////////////////














/////////////////////////////////////////////////////////////////////////////////////
/*-------- NTP subroutines ----------*/
/////////////////////////////////////////////////////////////////////////////////////
#define NTP_PACKET_SIZE 48 // NTP time is in the first 48 bytes of message
/////////////////////////////////////////////////////////////////////////////////////
time_t getNtpTime() {
	if (ntp_interrupt_in_progress == false) {
		ntp_interrupt_in_progress = true;
		Serial.println(GetTextFromFlashMemory(NTP_SEND_MESSAGE));
		while (Udp.parsePacket() > 0); // discard any previously received packets
		byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets 
		memset(packetBuffer, 0, NTP_PACKET_SIZE);
		packetBuffer[0] = 0b11100011;   // LI, Version, Mode
		packetBuffer[1] = 0;     // Stratum, or type of clock
		packetBuffer[2] = 6;     // Polling Interval
		packetBuffer[3] = 0xEC;  // Peer Clock Precision
		packetBuffer[12] = 49;
		packetBuffer[13] = 0x4E;
		packetBuffer[14] = 49;
		packetBuffer[15] = 52;
		Udp.beginPacket(timeServer, 123); //NTP requests are to port 123
		Udp.write(packetBuffer, NTP_PACKET_SIZE);
		Udp.endPacket();
		delay(1500);
		volatile time_t result = 0;
		volatile int size = Udp.parsePacket();
		if (size >= NTP_PACKET_SIZE) {
			Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
			volatile unsigned long secsSince1900;
			// convert four bytes starting at location 40 to a long integer
			secsSince1900 = (unsigned long)packetBuffer[40] << 24;
			secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
			secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
			secsSince1900 |= (unsigned long)packetBuffer[43];
			result = secsSince1900 - 2208988800UL + TIME_ZONE_INT * SECS_PER_HOUR;
			if (
				(
				(                                                                  // plati od
					(month(result) == DAYLIGHTSAVING_START_MONTH)                      // breznove
					&& (month(nextSunday(result)) == (DAYLIGHTSAVING_START_MONTH + 1))  // posledni nedele
					&&
					(
					(weekday(result) != 1)
						||
						(hour(result) >= DAYLIGHTSAVING_START_HOUR)                      // od dvou hodin rano
						)
					)
					|| (month(result) > DAYLIGHTSAVING_START_MONTH)                 // a v nasledujich mesicich
					)
				&&
				(
				(                                                                  // plati do
					(month(result) == DAYLIGHTSAVING_STOP_MONTH)                       // rijnove
					&& (month(nextSunday(result)) == DAYLIGHTSAVING_STOP_MONTH)       // posledni nedele
					)
					||
					(
					(month(result) == DAYLIGHTSAVING_STOP_MONTH)
						&& (month(nextSunday(result)) == (DAYLIGHTSAVING_STOP_MONTH + 1))
						&& (weekday(result) == 1)
						&& (hour(result) < DAYLIGHTSAVING_STOP_HOUR)
						)
					|| (month(result) < DAYLIGHTSAVING_STOP_MONTH)                  // a v predchozich mesicich
					)
				) {
				result = result + DAYLIGHTSAVING_TIMESHIFT * SECS_PER_HOUR;
				TIME_ZONE = TIME_ZONE_DST;
			}
			else {
				TIME_ZONE = TIME_ZONE_NODST;
			};
			NTPworking = true;
			Serial.print(GetTextFromFlashMemory(NTP_RESPONSES_MESSAGE));
			Serial.print(ComposeTimeStamp(TIME_SYSTEM_CLOCK | TIME_FORMAT_HUMAN, result));
			Serial.println();
			//// PROBLEMA CON EL PRINT
			Serial.print(GetTextFromFlashMemory(RTC_TIME_MESSAGE));
			Serial.println(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, 0));
		}
		else {
			NTPworking = false;
			Serial.println(GetTextFromFlashMemory(NTP_NO_RESPONSE_MESSAGE));
		}
		ntp_interrupt_in_progress = false;
		return result;
	};
};
/////////////////////////////////////////////////////////////////////////////////////
bool checkntp()
{
	Serial.print(GetTextFromFlashMemory(NTP_WAIT_MESSAGE));
	for (byte thisByte = 0; thisByte < 4; thisByte++) {
		Serial.print(timeServer[thisByte], DEC);
		if (thisByte < 3) {
			Serial.print(F("."));
		};
	};
	Serial.println(F(" ..."));
	setTime(getNtpTime());
	get_rtc_datum();
	get_rtc_time();
	time_t avr_now = now();
	setTime(rtc_hour, rtc_minute, rtc_second, rtc_day, rtc_month, rtc_year);
	time_t rtc_now = now();
	setTime(avr_now);
	if (rtc_now == avr_now) {
		Serial.println(GetTextFromFlashMemory(RTC_OK_MESSAGE));
	}
	else {
		if (NTPworking) {
			rtc_second = second();
			rtc_minute = minute();
			rtc_hour = hour();
			rtc_weekday = weekday();
			rtc_day = day();
			rtc_month = month();
			rtc_year = year();
			if (!set_rtc_time()) {
				Serial.println(GetTextFromFlashMemory(RTC_TF_MESSAGE));
			}
			else {
				if (!set_rtc_datum()) {
					Serial.println(GetTextFromFlashMemory(RTC_DF_MESSAGE));
				}
				else {
					Serial.println(GetTextFromFlashMemory(RTC_SYNC_MESSAGE));
				};
			};
			return true;
		}
		else {
			Serial.println(GetTextFromFlashMemory(ERROR_MESSAGE));
			return false;
		};
	};
};
/////////////////////////////////////////////////////////////////////////////////////
/*-------- DHCP subroutine ----------*/
/////////////////////////////////////////////////////////////////////////////////////
bool checkdhcp() {
#if defined(ETHERNET_IS_PRESENT)
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.fillScreen(ST7735_BLACK);
	tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
	tft.setTextSize(1);
	tft.setCursor(0, 0);
#endif
	//PRINT PROBLEMA
	MySerialPrint(" ");
	MyPrint(GetTextFromFlashMemory(MAC_MESSAGE));
	MySerialPrint(" = ");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.println();
#endif
	for (byte thisByte = 0; thisByte < 6; thisByte++) {
		if (MACAddress[thisByte] < 16) {
			MyPrint("0");
		};
		String hexString = String(MACAddress[thisByte], HEX);
		hexString.toUpperCase();
		MyPrint(hexString);
		if (thisByte < 5) {
			MyPrint(":");
		};
	};
	MyPrintLn("");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
	tft.println();
#endif
	MyPrintLn(GetTextFromFlashMemory(DHCP_WAIT_MESSAGE));
	if (Ethernet.begin(MACAddress) == 0) {
		MyPrintLn(GetTextFromFlashMemory(DHCP_FAIL_MESSAGE));
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(10000);
		tft.fillScreen(ST7735_BLACK);
#endif
		return false;
	}
	else {
		MyPrintLn(GetTextFromFlashMemory(DHCP_PASS_MESSAGE));
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(2000);
		tft.fillScreen(ST7735_BLACK);
		tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
		tft.setTextSize(1);
		tft.setCursor(0, 0);
#endif
		MyPrint(GetTextFromFlashMemory(IP_MESSAGE));
		MySerialPrint(" = ");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.localIP()[thisByte], DEC));
			if (thisByte < 3) {
				MyPrint(".");
			};
		};
		MyPrintLn("");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif

		MyPrint(GetTextFromFlashMemory(MASK_MESSAGE));
		MySerialPrint(" = ");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.subnetMask()[thisByte], DEC));
			if (thisByte < 3) {
				MyPrint(".");
			};
		};
		MyPrintLn("");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif

		MyPrint(GetTextFromFlashMemory(GATEWAY_MESSAGE));
		MySerialPrint(" = ");
		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.gatewayIP()[thisByte], DEC));
			if (thisByte < 3) {
				MyPrint(".");
			};
		};
		MyPrintLn("");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif

		MyPrint(GetTextFromFlashMemory(DNS_MESSAGE));
		MySerialPrint(" = ");
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.println();
#endif
		for (byte thisByte = 0; thisByte < 4; thisByte++) {
			MyPrint(String(Ethernet.dnsServerIP()[thisByte], DEC));
			if (thisByte < 3) {
				MyPrint(".");
			};
		};
		MyPrint("\r\n");

		Udp.begin(LOCALUDPPORT);
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(10000);
		tft.fillScreen(ST7735_BLACK);
#endif
		return true;
	};
#else
	return false;
#endif
};





/*

bool checkdhcp() {
	Serial.println(GetTextFromFlashMemory(DHCP_WAIT_MESSAGE));
	if (Ethernet.begin(MACAddress) == 0)
	{
		Serial.println(GetTextFromFlashMemory(DHCP_FAIL_MESSAGE));
		return false;
	}
	else {
		Serial.println(GetTextFromFlashMemory(DHCP_PASS_MESSAGE));
		Serial.print(GetTextFromFlashMemory(IP_MESSAGE));
		Serial.println(Ethernet.localIP());
		Serial.print(GetTextFromFlashMemory(MASK_MESSAGE));
		Serial.println(Ethernet.subnetMask());
		Serial.print(GetTextFromFlashMemory(GATEWAY_MESSAGE));
		Serial.println(Ethernet.gatewayIP());
		Serial.print(GetTextFromFlashMemory(DNS_MESSAGE));
		Serial.println(Ethernet.dnsServerIP());
		Serial.print(GetTextFromFlashMemory(MAC_MESSAGE));
		MySerialPrint(" = ");
		
		
		
		
		
		for (byte thisByte = 0; thisByte < 6; thisByte++) {
			if (MACAddress[thisByte] < 16) {
				Serial.print(F("0"));
			};
			Serial.print(MACAddress[thisByte], HEX);
			if (thisByte < 5) {
				Serial.print(F(":"));
			};
		};
		
		Serial.println();
		Udp.begin(LOCALUDPPORT);
		return true;
	};
};
*/

void ShowTime(bool ForceShowTemperature) {/////////////////////////////////////////////////////////////////////////////////////
	if (time_last_lcd + LCD_UPDATE_PERIOD <= now()) {
		time_last_lcd = now();
		String PrtString;
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
		tft.setTextSize(2);
		tft.setCursor(21, 0);
		PrtString = ComposeTimeStamp(TIME_SYSTEM_CLOCK bitor TIME_FORMAT_DATE_ONLY, 0) + "  ";
		tft.print(PrtString);

		tft.setTextSize(3);
		tft.setCursor(9, 53);
		PrtString = ComposeTimeStamp(TIME_SYSTEM_CLOCK bitor TIME_FORMAT_TIME_ONLY, 0);
		tft.print(PrtString);
#endif

		if ((time_last_reportDS + REPORTDS_TIME_PERIOD <= now()) || (ForceShowTemperature)) {
			time_last_reportDS = now();

#if defined(ADAFRUIT_ST7735_IS_PRESENT)
			tft.setTextSize(3);
			tft.setCursor(18, 107);
			PrtString = ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, false);
			tft.print(PrtString);
#endif
			//order to avoid distortion running NTP process console print is omited
			if (!NTPasked) {
				MySerialPrint(GetTextFromFlashMemory(INFO_MESSAGE));
				MySerialPrint(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, 0));
				MySerialPrint(" ");
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, false));

#if defined(TMP36_IS_PRESENT)
				Serial.println();
				MySerialPrint(GetTextFromFlashMemory(SENSORS_MESSAGE));
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_TMP36_1, false));
				MySerialPrint(" ");
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_TMP36_2, false));
				//MySerialPrint("\r\n");
#endif
				

#if defined(DHT22_IS_PRESENT)
				Serial.println();
				MySerialPrint(GetTextFromFlashMemory(SENSORS_MESSAGE_2));
				MySerialPrint(ComposeTemperatureString(TEMPERATURE_SENSOR_DHT22, false) +" "+ ComposeHumedityString(TEMPERATURE_SENSOR_DHT22,false));
				//MySerialPrint(" ");
				MySerialPrint("\r\n");
				MySerialPrint("Remoto:");
				MySerialPrint(ComposeTemperatureString(REMOTE_TEMPERATURE_SENSOR_DHT22, false));
				MySerialPrint("\r\n");

#endif
				
				MySerialPrint("\r\n");


#if defined(LCD16x2_IS_PRESENT)
				lcd.setCursor(0, 0);
				lcd.print(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_TIME_ONLY, 0) + " " + ComposeTemperatureString(TEMPERATURE_SENSOR_DS3231, true));
				lcd.setCursor(0, 1);
				lcd.print(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_DATE_ONLY, 0)); //TIME_FORMAT_DATE_ONLY
#endif
			};
		};
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
		delay(490);
		tft.setTextSize(3);
		tft.setCursor(9, 53);
		PrtString = ComposeTimeStamp(TIME_SYSTEM_CLOCK bitor TIME_FORMAT_TIME_ONLY_WO_COLON, 0);
		tft.print(PrtString);
#endif
	};
};



bool  MaintainTimeSources(bool force) {/////////////////////////////////////////////////////////////////////////////////////
	String OutString = "";
	if ((NTPasked) || (force)) {
		NTPasked = false;
		time_t avr_now = 0;
		time_t timediff = 0;
		if (!read_rtc(rtc_now)) {
			rtc_now = 0;
		};
		avr_now = now();
		if (NTPworking) {
			MySerialPrint(GetTextFromFlashMemory(NTP_RESPONSES_MESSAGE));
			MySerialPrint(ComposeTimeStamp(TIME_SYSTEM_CLOCK | TIME_FORMAT_HUMAN, avr_now));
			MySerialPrint(", SYS Epoch: " + String(avr_now) + "\r\n");
		}
		else {
			MySerialPrint(GetTextFromFlashMemory(NTP_ERROR_MESSAGE) + "\r\n");
			return false;
		};
		if (RTCworking) {
			MySerialPrint(GetTextFromFlashMemory(RTC_TIME_MESSAGE));
			MySerialPrint(ComposeTimeStamp(TIME_RTC_CLOCK | TIME_FORMAT_HUMAN, rtc_now));
			MySerialPrint(", RTC Epoch: " + String(rtc_now) + "\r\n");
			String strSign = "";
			if (rtc_now > avr_now) {
				timediff = rtc_now - avr_now;
				strSign = "+";
			}
			else {
				timediff = avr_now - rtc_now;
				strSign = "-";
			};
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
#define ST7735_SILVER		0xC618
#define ST7735_GRAY		0x8410
			tft.setTextSize(1);
			tft.setCursor(0, 79);
			tft.setTextColor(ST7735_BLACK, ST7735_BLACK);
			tft.print(LastOutString);
			tft.setCursor(0, 79);
			tft.setTextColor(ST7735_GRAY, ST7735_BLACK);
#endif
			if (timediff == 0) {
				OutString = GetTextFromFlashMemory(RTC_OK_MESSAGE);
				MyPrintLn(OutString);
			}
			else {
				OutString = GetTextFromFlashMemory(RTC_NEED_SYNC_MESSAGE) + strSign + String(timediff) + GetTextFromFlashMemory(SECONDS_MESSAGE);
				MyPrintLn(OutString);
#if defined(ENABLE_RTC_UPDATE)
				rtc_second = second();
				rtc_minute = minute();
				rtc_hour = hour();
				rtc_weekday = weekday();
				rtc_day = day();
				rtc_month = month();
				rtc_year = year();
				if (!set_rtc_time()) {
					MySerialPrint(GetTextFromFlashMemory(RTC_TF_MESSAGE) + "\r\n");
				}
				else {
					if (!set_rtc_datum()) {
						MySerialPrint(GetTextFromFlashMemory(RTC_DF_MESSAGE) + "\r\n");
					}
					else {
						MySerialPrint(GetTextFromFlashMemory(RTC_SYNC_MESSAGE) + "\r\n");
					};
				};
#endif    
			};
#if defined(ADAFRUIT_ST7735_IS_PRESENT)
			tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
			LastOutString = OutString;
#endif
		}
		else {
			MySerialPrint(GetTextFromFlashMemory(RTC_NOT_PRESENT) + "\r\n");
		};
		return true;
	};
	return false;
};

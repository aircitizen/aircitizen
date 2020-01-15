// Aircitizen - station base Lotus
//
// Authors: Bao Lam Le & Vincent Dupuis
// License CC BY-NC
//
// DHT22 : D7
// GPS : D2
// PMS : D6
// OLED : I2C
// Multichanel gas sensor I2C !!!! CHECK I2C ADRESS! 

// revision
// change pb time BaoLam
// change DHT lib
// add particle numbers for PMS5003

// filename
const char Filename[] = "AirCitizen_5.csv";

// libraries
// GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// DHT Temp & Hum sensor
#include "DHT.h"

// multichannel gas
#include "MutichannelGasSensor.h"


// OLED I2C display
#include <Wire.h>
#include "SeeedOLED.h"
#include <avr/pgmspace.h>

// SD card 
#include <SPI.h>
#include <SdFat.h>

//Variables
int chk;

//#define debuggEnabled

dht DHT;
#define DHT22_PIN 7

#define RXPin 2      // 
#define TXPin 3      
#define GPSBaud  9600

/*Indicator*/
int INDIC = 0;


/*PMS5003*/
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

/*Sensor*/
float lastTemperature = 0;
float lastHumidity = 0;
float lastPM1 = 0;
float lastPM25 = 0;
float lastPM10 = 0;
float lastCO = 0;
float lastNO2 = 0;
// particle numbers
int lastN_03 = 0;
int lastN_05 = 0;
int lastN_1 = 0;
int lastN_25 = 0;
int lastN_5 = 0;
int lastN_10 = 10;


/* Pour l'utilisation de U8glib*/
const uint8_t lineSpacing = 2;
const uint8_t XStartPosition = 0;
const uint8_t YStartPosition = 0;

/* GPS_valid */
bool GPS_valid = false;
bool GPS_GOTDATA = true;
double GPS_longitude = 0;
double GPS_latitude = 0;
/* SD file */
SdFat sd;
SdFile myFile;
// SD card chip select pin.
const uint8_t SD_CS_PIN = 4;
boolean csvInit = false;
  /* Time and date prefix */
#define dateBuf_length        11
static char dateBuf[dateBuf_length] = {'0','0','/','0','0','/','2','0','1','6','\0'};

#define timeBuf_length        9
static char timeBuf[timeBuf_length] = {'0','0',':','0','0',':','0','0','\0'};

//uint8_t bits[5];  // buffer to receive data of dht22

#define TIMEOUT 10000

static const char* DATA_UPDATE[18]={
  "GPS",
  "UTC Date",
  "UTC Time", 
  "Latitude",
  "Longitude",
  "Temp",
  "Humid",
  "PM1",
  "PM2.5",
  "PM10",
  "CO",
  "NO2",
  "N_0.3",
  "N_0.5",
  "N_1.0",
  "N_2.5",
  "N_5.0",
  "N_10.0"
};

static const char* UNITS[18]={
  "",
  "",
  "",
  "",
  "",
  "C",
  "%",
  "ug/m3",
  "ug/m3",
  "ug/m3",
  "ppm",
  "ppb",
  "",
  "",
  "",
  "",
  "",
  ""
};

// The TinyGPS++ object
TinyGPSPlus tinyGPS;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
#define gpsPort ss

// Plantower on sspms
SoftwareSerial sspms(6, 7);


// function draw()

void draw(){

  void *OledData_buf[11] = {dateBuf, timeBuf, &GPS_latitude, &GPS_longitude, &lastTemperature, &lastHumidity, &lastPM1, &lastPM25, &lastPM10, &lastCO, &lastNO2};

  SeeedOled.clearDisplay();          //clear the screen and set start position to top left corner

  SeeedOled.setTextXY(0,0);  
  //SeeedOled.setGrayLevel(15); //Set Grayscale level. Any number between 0 - 15.
  SeeedOled.putString(DATA_UPDATE[0]); SeeedOled.putString(": ");
  if (GPS_valid){
    SeeedOled.putString("o");
  }
  else {
    SeeedOled.putString("x");
  }

  //u8g.print("       ");
  SeeedOled.putString("       ");
  if (INDIC==1) {
    SeeedOled.putString("-");
    }
  else {
    SeeedOled.putString("|");
  }
  // Date and time
  //SeeedOled.setTextXY(1, 0);
  //SeeedOled.putString((char*)OledData_buf[1]);
  //SeeedOled.setTextXY(2, 0);
  //SeeedOled.putString((char*)OledData_buf[2]);
  // RH and Temperature
  //SeeedOled.setTextXY(3, 0);
  //SeeedOled.putString("T=");SeeedOled.putNumber(*((float*)OledData_buf[4])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[5]);  
  //SeeedOled.putString(" ");
  //SeeedOled.putString("RH=");SeeedOled.putNumber(*((float*)OledData_buf[5])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[6]);
  //SeeedOled.putString(" ");
  
 SeeedOled.setTextXY(2,0);
  SeeedOled.putString(DATA_UPDATE[7]); SeeedOled.putString(":   ");  
  SeeedOled.putNumber(*((float*)OledData_buf[6])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[7]);

  SeeedOled.setTextXY(3, 0);
  SeeedOled.putString(DATA_UPDATE[8]); SeeedOled.putString(": ");  
  SeeedOled.putNumber(*((float*)OledData_buf[7])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[8]);

  SeeedOled.setTextXY(4,0);
  SeeedOled.putString(DATA_UPDATE[9]); SeeedOled.putString(":  ");  
  SeeedOled.putNumber(*((float*)OledData_buf[8])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[9]);

  SeeedOled.setTextXY(5,0);
  SeeedOled.putString(DATA_UPDATE[10]); SeeedOled.putString(":  ");  
  SeeedOled.putNumber(*((float*)OledData_buf[9])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[10]);    

  SeeedOled.setTextXY(6,0);
  SeeedOled.putString(DATA_UPDATE[11]); SeeedOled.putString(":  ");  
  SeeedOled.putNumber(*((float*)OledData_buf[10])); SeeedOled.putString(" "); SeeedOled.putString(UNITS[11]);  
}

void setup() {
  Serial.begin(9600);
  // OLED
  Wire.begin();
  SeeedOled.init();             //initialize SEEED OLED display
  SeeedOled.clearDisplay();     //Clear Display.
  SeeedOled.setNormalDisplay(); //Set Normal Display Mode
  //SeeedOled.setVerticalMode();  // Set to vertical mode for displaying text
  
  // GPS
  ss.begin(GPSBaud);
  delay(100);
  //PMS on sspms;
  sspms.begin(9600);
  delay(100);
  ss.listen();
  // DHT
  //dht.begin();
 
  if (!sd.begin(SD_CS_PIN)) {
  #ifdef debuggEnabled
      Serial.println(F("initialization failed!"));
  #endif 
      return;
    }
   
 
  #ifdef debuggEnabled
  Serial.println(F("initialization done."));   
  #endif 
  if (!sd.exists(Filename)) {
    #ifdef debuggEnabled
      Serial.println(F("Creating AirCitizen.csv..."));
    #endif 
    myFile.open(Filename, FILE_WRITE);
    myFile.close();
    delay(1000);
    txHeader();
    
  } else{
    #ifdef debuggEnabled
      Serial.println(F("AirCitizen.csv exists ..."));
    #endif 
  }   

    // CHECK I2C ADDRESS !!! 2 ercicka 1 transp
    gas.begin(0x02);//the default I2C address of the slave is 0x04
    gas.powerOn();
    #ifdef debuggEnabled
      Serial.print("Firmware Version = ");
      Serial.println(gas.getVersion());
    #endif 

}
// ----------------------------------------------------------------------------------------------------------------
void loop() {
  /*
  // picture loop
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage());   
  */
  ss.listen();
  delay(100);
  // rebuild the picture after some delay
  if(GPS_GOTDATA) {
    feedGPS();
    GPS_GOTDATA = false;
  }
  sckDHT22();
  sspms.listen();
  delay(100);
  sckPMS();
  ss.listen();
  sckGAS();
  txSD();

  if (INDIC==1) {INDIC = 0;}
  else {INDIC = 1;}
  
  draw();  
  smartDelay(1000);
  
}
// ----------------------------------------------------------------------------------------------------------------

static void feedGPS() {
  /*Date et time */
  uint8_t Hour = 12;
  uint8_t Minute = 50;
  uint8_t Second = 33;
  uint16_t Year = 2012;
  uint8_t Month = 5;
  uint8_t Day = 5;
  if (tinyGPS.location.isValid()){
    GPS_valid = true; 
    GPS_longitude = tinyGPS.location.lat();
    GPS_latitude =  tinyGPS.location.lng();
  } else {
    GPS_valid = false; 
  }
  if(tinyGPS.date.isValid()){
    Year = tinyGPS.date.year();
    Month = tinyGPS.date.month();
    Day = tinyGPS.date.day();
    sckDate(Day,Month,Year);
  }
  if (tinyGPS.time.isValid()) {
    Hour = tinyGPS.time.hour();
    Minute = tinyGPS.time.minute();
    Second = tinyGPS.time.second();
    sckTime(Second, Minute, Hour);
  }
  #ifdef debuggEnabled
  Serial.print("feed GPS OK");
  #endif
}

// This custom version of delay() ensures that the tinyGPS object
// is being "fed". From the TinyGPS++ examples.
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort.available())
      if(tinyGPS.encode(gpsPort.read())) {
        GPS_GOTDATA = true; // Send it to the encode function
      }
  // tinyGPS.encode(char) continues to "load" the tinGPS object with new
  // data coming in from the GPS module. As full NMEA strings begin to come in
  // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

boolean sckDHT22()
{
  int chk = DHT.read22(DHT22_PIN);
  // READ DATA
  float hum = DHT.humidity;
  float temp = DHT.temperature;
  //float h = dht.readHumidity();
  //float t = dht.readTemperature();
  #ifdef debuggEnabled
    Serial.println("");
    Serial.print("Humidity: "); 
    Serial.print(hum);
    Serial.println(" %");
    Serial.print("Temperature: "); 
    Serial.print(temp);
    Serial.println(" *C");
  #endif
  lastTemperature = temp;
  lastHumidity = hum;

  return true;
}


void sckPMS()
{   
    if(sspms.find(0x42)){    
    sspms.readBytes(buf,LENG);
    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        lastPM1 = (buf[9]<<8) + buf[10];
        lastPM25 = (buf[11]<<8) + buf[12];
        lastPM10 = (buf[13]<<8) + buf[14];
        lastN_03=(buf[15]<<8) + buf[16];
        lastN_05=(buf[17]<<8) + buf[18];
        lastN_1=(buf[19]<<8) + buf[20];
        lastN_25=(buf[21]<<8) + buf[22];
        lastN_5=(buf[23]<<8) + buf[24];
        lastN_10=(buf[25]<<8) + buf[26];
        //transmitPM01(buf); //count PM1.0 value of the air detector module
        //transmitPM2_5(buf);//count PM2.5 value of the air detector module
        //transmitPM10(buf); //count PM10 value of the air detector module
      }           
    } 
  }
  #ifdef debuggEnabled
        Serial.print("PM1_0: ");  
        Serial.print(lastPM1);
        Serial.println("  ug/m3");            
        Serial.print("PM2_5: ");  
        Serial.print(lastPM25);
        Serial.println("  ug/m3");     
        Serial.print("PM10: ");  
        Serial.print(lastPM10);
        Serial.println("  ug/m3");   
        Serial.println();
  #endif
      
}

void sckGAS()
{   
  lastCO = gas.measure_CO();
  #ifdef debuggEnabled
    Serial.print("The concentration of CO is ");
    if(lastCO>=0) Serial.print(lastCO);
    else Serial.print("invalid");
    Serial.println(" ppm");
  #endif 
  lastNO2 = 1000.0*gas.measure_NO2();
  #ifdef debuggEnabled
    Serial.print("The concentration of NO2 is ");
    if(lastNO2>=0) Serial.print(lastNO2);
    else Serial.print("invalid");
    Serial.println(" ppm");
  #endif 
}

void txSD() {
  #ifdef debuggEnabled
  Serial.println("*** txSD ***");
  #endif
  
  // if the file opened okay, write to it:
  
  if (myFile.open(Filename, FILE_WRITE)) {
  #ifdef debuggEnabled
      Serial.println(F("Writing...")); 
  #endif 
  myFile.print(GPS_valid);
  myFile.print(",");
  myFile.print(dateBuf);
  myFile.print(",");  
  myFile.print(timeBuf);
  myFile.print(",");
  myFile.print(GPS_longitude,7);
  myFile.print(",");
  myFile.print(GPS_latitude,7);
  myFile.print(",");
  myFile.print(lastTemperature);
  myFile.print(",");
  myFile.print(lastHumidity);
  myFile.print(",");
  myFile.print(lastPM1);
  myFile.print(",");
  myFile.print(lastPM25);
  myFile.print(",");
  myFile.print(lastPM10);
  myFile.print(",");
  myFile.print(lastCO);
  myFile.print(",");
  myFile.print(lastNO2);
  myFile.print(",");
  myFile.print(lastN_03);
  myFile.print(",");
  myFile.print(lastN_05);
  myFile.print(",");
  myFile.print(lastN_1);
  myFile.print(",");
  myFile.print(lastN_25);
  myFile.print(",");
  myFile.print(lastN_5);
  myFile.print(",");
  myFile.print(lastN_10);
  myFile.println();
  
  // close the file:
  myFile.close();

    
//#ifdef debuggEnabled
  Serial.print(GPS_valid);
  Serial.print(",");
  Serial.print(dateBuf);
  Serial.print(",");  
  Serial.print(timeBuf);
  Serial.print(",");
  Serial.print(GPS_longitude,7);
  Serial.print(",");
  Serial.print(GPS_latitude,7);
  Serial.print(",");
  Serial.print(lastTemperature);
  Serial.print(",");
  Serial.print(lastHumidity);
  Serial.print(",");
  Serial.print(lastPM1);
  Serial.print(",");
  Serial.print(lastPM25);
  Serial.print(",");
  Serial.print(lastPM10);
  Serial.print(",");
  Serial.print(lastCO);
  Serial.print(",");
  Serial.print(lastNO2);
  Serial.print(",");
  Serial.print(lastN_03);
  Serial.print(",");
  Serial.print(lastN_05);
  Serial.print(",");
  Serial.print(lastN_1);
  Serial.print(",");
  Serial.print(lastN_25);
  Serial.print(",");
  Serial.print(lastN_5);
  Serial.print(",");
  Serial.print(lastN_10);
  Serial.println();
  Serial.println(F("Closing...")); 
//#endif 
  }
}

void txHeader() { 
  #ifdef debuggEnabled
  Serial.println("*** txHeader ***");
  #endif
  // if the file opened okay, write to it:
  if (myFile.open(Filename, FILE_WRITE)) {
  #ifdef debuggEnabled
      Serial.println(F("Writing...")); 
  #endif 
    for (uint8_t i=0; i<17; i++)
    {
      myFile.print(DATA_UPDATE[i]);
      myFile.print(", ");
    }
    myFile.print(DATA_UPDATE[18]);
    myFile.println();
    // close the file:
    myFile.close();
    #ifdef debuggEnabled
        Serial.println(F("Closing...")); 
    #endif 
  }
}

char* sckDate(uint8_t dayin, uint8_t monthin, uint16_t yearin) {
  char zero = '0';
  dateBuf[0] = (dayin / 10) + zero;
  dateBuf[1] = (dayin % 10) + zero;
  dateBuf[3] = (monthin / 10) + zero;
  dateBuf[4] = (monthin % 10) + zero;
  dateBuf[8] = (yearin % 100)/10 + zero;
  dateBuf[9] = (yearin % 10) + zero;
  return dateBuf;
}

char* sckTime(uint8_t secondein, uint8_t minutein, uint8_t hourin) {
  char zero = '0';
  timeBuf[0] = (hourin / 10) + zero;
  timeBuf[1] = (hourin % 10) + zero;
  timeBuf[3] = (minutein / 10) + zero;
  timeBuf[4] = (minutein % 10) + zero;
  timeBuf[6] = (secondein / 10) + zero;
  timeBuf[7] = (secondein % 10) + zero;
  return timeBuf;
}

// PMS functions
char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}


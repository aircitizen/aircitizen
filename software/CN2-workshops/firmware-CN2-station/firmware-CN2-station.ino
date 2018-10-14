// Aircitizen - CN2 station
//
// Authors: Bao Lam Le & Vincent Dupuis
// License CC BY-NC
//

// libraries
// GPS
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

// DHT Temp & Hum sensor
#include "DHT.h"

// OLED I2C display
#include <Wire.h>
#include "U8glib.h"

// SD card 
#include <SPI.h>
#include <SdFat.h>

#define DEBUG

union _uintToChar { // Utilitaire de conversion
  unsigned int f;
  char c[2];
} uintToChar;


//Variables
int chk;

#define debuggEnabled

U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 

#define DHTPIN A0     // DHT pin
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

#define RXPin 10      // 
#define TXPin 11      
#define GPSBaud  9600

/*Indicator*/
int INDIC = 0;


/*PMS3003*/
#define LENG 23   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];

int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module
int O3Value = 0;
int NO2Value = 0;

/*Sensor*/
float lastTemperature = 0;
float lastHumidity = 0;
float lastPM1 = 0;
float lastPM25 = 0;
float lastPM10 = 0;
float lastO3 = 0;
float lastNO2 = 0;


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
const uint8_t SD_CS_PIN = 53; //4;
boolean csvInit = false;
const char Filename[] = "AirCitizen_1.csv";
  /* Time and date prefix */
#define dateBuf_length        11
static char dateBuf[dateBuf_length] = {'0','0','/','0','0','/','2','0','1','6','\0'};

#define timeBuf_length        9
static char timeBuf[timeBuf_length] = {'0','0',':','0','0',':','0','0','\0'};

uint8_t bits[5];  // buffer to receive data of dht22

#define TIMEOUT 10000

static const char* DATA_UPDATE[12]={
  "GPS_Valid",
  "UTC Date",
  "UTC Time", 
  "Latitude",
  "Longitude",
  "Temp",
  "Humid",
  "PM1",
  "PM2.5",
  "PM10",
  "O3",
  "NO2"
};

static const char* UNITS[12]={
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
  "ppb",
  "ppb"
};

// The TinyGPS++ object
TinyGPSPlus tinyGPS;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
#define gpsPort ss

// Plantower on Serial1

unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 3000;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;

void draw(){
   u8g.setFont(u8g_font_fixed_v0); //u8g_font_6x10);
  uint8_t sautligne = u8g.getFontLineSpacing();
  
  
  void *OledData_buf[11] = {dateBuf, timeBuf, &GPS_latitude, &GPS_longitude, &lastTemperature, &lastHumidity, &lastPM1, &lastPM25, &lastPM10, &lastO3, &lastNO2};
  
  u8g.setPrintPos(XStartPosition, YStartPosition + sautligne);
  u8g.print(DATA_UPDATE[0]); u8g.print(": "); u8g.print(GPS_valid);

  u8g.print("       ");
  if (INDIC==1) {
    u8g.print("-");
    }
  else {
    u8g.print("|");
  }
  // Date and time
  u8g.setPrintPos(XStartPosition, YStartPosition + sautligne*2);
  u8g.print((char*)OledData_buf[1]);
  //u8g.setPrintPos(XStartPosition, YStartPosition + sautligne*3);
  //u8g.print((char*)OledData_buf[2]);
  // RH and Temperature
  u8g.setPrintPos(XStartPosition, YStartPosition + sautligne*4);
  u8g.print("T=");u8g.print(*((float*)OledData_buf[4]),1); u8g.print(" "); u8g.print(UNITS[5]);  
  u8g.print(" ");
  u8g.print("RH=");u8g.print(*((float*)OledData_buf[5]),1); u8g.print(" "); u8g.print(UNITS[6]);
  u8g.print(" ");
  
  u8g.setPrintPos(XStartPosition, YStartPosition + sautligne*5);
  u8g.print(DATA_UPDATE[7]); u8g.print(":    ");  
  u8g.print(*((float*)OledData_buf[6]),0); u8g.print(" "); u8g.print(UNITS[7]);

  u8g.setPrintPos(XStartPosition, YStartPosition + sautligne*6);
  u8g.print(DATA_UPDATE[8]); u8g.print(": ");  
  u8g.print(*((float*)OledData_buf[7]),0); u8g.print(" "); u8g.print(UNITS[8]);

  u8g.setPrintPos(XStartPosition, YStartPosition + sautligne*7);
  u8g.print(DATA_UPDATE[9]); u8g.print(":  ");  
  u8g.print(*((float*)OledData_buf[8]),0); u8g.print(" "); u8g.print(UNITS[9]);  
}

void setup() {
  Serial.begin(9600);
  // GPS
  ss.begin(GPSBaud);
  delay(1000);
  //PMS on Serial1;
  Serial1.begin(9600);
  delay(1000);
  // DHT
  dht.begin();
 
  if (!sd.begin(SD_CS_PIN)) {
  #ifdef debuggEnabled
      Serial.println(F("initialization failed!"));
  #endif 
      return;
    }
   
   // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  
  u8g.setFont(u8g_font_unifont);
  u8g.setColorIndex(1); // Instructs the display to draw with a pixel on. 
  
  /* Shinyei initialisation */
  //pinMode(SHINYEI_PIN, INPUT);
  starttime = millis();

  /* Sharp initialisation */
  //pinMode(LED_POWER_PIN, OUTPUT); // Initialisation du capteur de particules fines
  //pinMode(DUST_SENSOR_PIN, INPUT);

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

}

void loop() {
  /*
  // picture loop
  u8g.firstPage();  
  do {
    draw();
  } while( u8g.nextPage());   
  */
  // rebuild the picture after some delay
  if(GPS_GOTDATA) {
    feedGPS();
    GPS_GOTDATA = false;
  }
  sckDHT22();
  sckPMS();
  txSD();

  if (INDIC==1) {INDIC = 0;}
  else {INDIC = 1;}
  
  u8g.firstPage();
  draw();
  
  do {  
    draw();
  } while( u8g.nextPage() );
  //delay(1000);   
  
  smartDelay(1000);
  
}

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
    GPS_longitude = tinyGPS.location.lng();
    GPS_latitude =  tinyGPS.location.lat();
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
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  #ifdef debuggEnabled
    Serial.println("");
    Serial.print("Humidity: "); 
    Serial.print(h);
    Serial.println(" %");
    Serial.print("Temperature: "); 
    Serial.print(t);
    Serial.println(" *C");
  #endif
  lastTemperature = t;
  lastHumidity = h;

  return true;
}

void sckPMS()
{   
    if(Serial1.find(0x42)){    
    Serial1.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        lastPM1 = PM01Value;
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        lastPM25 = PM2_5Value;
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
        lastPM10 = PM10Value;
      }           
    } 
  }

        Serial.print("PM1_0: ");  
        Serial.print(PM01Value);
        Serial.println("  ug/m3");            
        Serial.print("PM2_5: ");  
        Serial.print(PM2_5Value);
        Serial.println("  ug/m3");     
        Serial.print("PM10: ");  
        Serial.print(PM10Value);
        Serial.println("  ug/m3");   
        Serial.println();

      
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
  void *SdData_buf[11] = {&GPS_valid, dateBuf, timeBuf, &GPS_longitude, &GPS_latitude, &lastTemperature, &lastHumidity, &lastPM1, &lastPM25, &lastPM10, &lastO3};
  
  for (uint8_t i=0; i<11; i++) {
    if (i>2) {
      myFile.print(*((float*)SdData_buf[i]), 7);
    } else if (i == 0) {
      myFile.print(*((bool*)SdData_buf[i]));
    } else {
      myFile.print((char*)SdData_buf[i]);
    }
    myFile.print(",");
  }
    myFile.print(lastNO2,6);
    myFile.println();
    // close the file:
    myFile.close();
#ifdef debuggEnabled
    for (uint8_t i=0; i<11; i++) {
    if (i>2) {
      Serial.print(*((float*)SdData_buf[i]), 6);
    } else if (i == 2) {
      Serial.print(*((bool*)SdData_buf[i]));
    } else {
      Serial.print((char*)SdData_buf[i]);
    }
    Serial.print(";");
  }
    Serial.print(lastNO2,6);
    Serial.println();
    Serial.println(F("Closing...")); 
#endif 
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
    for (uint8_t i=0; i<11; i++)
    {
      myFile.print(DATA_UPDATE[i]);
      myFile.print(", ");
    }
    myFile.print(DATA_UPDATE[11]);
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
  timeBuf[7] = (secondein / 10) + zero;
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

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}


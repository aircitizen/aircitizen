#include "Arduino.h"
#include "serialReadPMValue.h"

uint16_t PM01Value=0;          //define PM1.0 value of the air detector module
uint16_t PM2_5Value=0;         //define PM2.5 value of the air detector module
uint16_t PM10Value=0;         //define PM10 value of the air detector module


#define receiveDatIndex 24

uint8_t receiveDat[receiveDatIndex]; //receive data from the air detector module


void setup() {
  
  // put your setup code here, to run once:
Serial.begin(115200);   //set the serial's Baudrate of the air detector module
Serial1.begin(9600);   //set the serial1's Baudrate of the air detector module

}



void loop()
{

 int length = serialRead(Serial1,receiveDat,receiveDatIndex,5);
 int checkSum=checkValue(receiveDat,receiveDatIndex);
 
  if(length&&checkSum)
  {

    PM01Value=transmitPM01(receiveDat); //count PM1.0 value of the air detector module

    PM2_5Value=transmitPM2_5(receiveDat);//count PM2.5 value of the air detector module

    PM10Value=transmitPM10(receiveDat); //count PM10 value of the air detector module

  }
    static unsigned long OledTimer=millis();       //every 0.5s update the temperature and humidity from DHT11 sensor
    if (millis() - OledTimer >=1000) 
    {
      OledTimer=millis(); 
      
      Serial.print("PM1.0: ");  //send PM1.0 data to bluetooth
      Serial.print(PM01Value);
      Serial.println("  ug/m3");            
    
      Serial.print("PM2.5: ");  //send PM1.0 data to bluetooth
      Serial.print(PM2_5Value);
      Serial.println("  ug/m3");     
      
      Serial.print("PM10:  ");  //send PM1.0 data to bluetooth
      Serial.print(PM10Value);
      Serial.println("  ug/m3");   
    }



 }

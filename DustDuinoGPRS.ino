// DUSTDUINO v1.0
// Released 5 June 2015
//
// This software is released as-is, without warranty,
// under a Creative Commons Attribution-ShareAlike
// 3.0 Unported license. For more information about
// this license, vis
// Available at: https://github.com/harlequin-tech/WiFlyHQ
#include <WiFlyHQ.h>
#include <avr/wdt.h>
//it:
// http://creativecommons.org/licenses/by-sa/3.0/
//
// Written by Guima San and Matthew Schroyer, except where specified.
// For more information on building a DustDuino, visit:
// http://www.mentalmunition.com/2013/10/measure-air-pollution-with-dustduino-of.html
//and http://codigourbano.org/primeiro-sensor-independente-para-monitorar-poluicao-e-instalado-em-sp/

#include <SoftwareSerial.h>
SoftwareSerial gprsSerial(7,8);

unsigned long starttime;

unsigned long triggerOnP1;
unsigned long triggerOffP1;
unsigned long pulseLengthP1;
unsigned long durationP1;
boolean valP1 = HIGH;
boolean triggerP1 = false;

unsigned long triggerOnP2;
unsigned long triggerOffP2;
unsigned long pulseLengthP2;
unsigned long durationP2;
boolean valP2 = HIGH;
boolean triggerP2 = false;

float ratioP1 = 0;
float ratioP2 = 0;
unsigned long sampletime_ms = 1800;
float countP1;
float countP2;

void setup()
{
  gprsSerial.begin(19200); // GPRS shield baud rate 
  Serial.begin(19200);   
  delay(500);
  pinMode(9, OUTPUT); //Start GPRS Shield
  digitalWrite(9, HIGH);
  delay(5000);

  
}
 
void loop()
{

  valP1 = digitalRead(3);
  valP2 = digitalRead(2);
  
  if(valP1 == LOW && triggerP1 == false){
    triggerP1 = true;
    triggerOnP1 = micros();
  }
  
  if (valP1 == HIGH && triggerP1 == true){
      triggerOffP1 = micros();
      pulseLengthP1 = triggerOffP1 - triggerOnP1;
      durationP1 = durationP1 + pulseLengthP1;
      triggerP1 = false;
  }
  
    if(valP2 == LOW && triggerP2 == false){
    triggerP2 = true;
    triggerOnP2 = micros();
  }
  
    if (valP2 == HIGH && triggerP2 == true){
      triggerOffP2 = micros();
      pulseLengthP2 = triggerOffP2 - triggerOnP2;
      durationP2 = durationP2 + pulseLengthP2;
      triggerP2 = false;
  }
  
    
    if ((millis() - starttime) > sampletime_ms) {
      
      ratioP1 = durationP1/(sampletime_ms*10.0);  // Integer percentage 0=>100
      ratioP2 = durationP2/(sampletime_ms*10.0);
      countP1 = 1.1*pow(ratioP1,3)-3.8*pow(ratioP1,2)+520*ratioP1+0.62;
      countP2 = 1.1*pow(ratioP2,3)-3.8*pow(ratioP2,2)+520*ratioP2+0.62;
      float PM10count = countP2;
      float PM25count = countP1 - countP2;
      
      // first, PM10 count to mass concentration conversion
      double r10 = 2.6*pow(10,-6);
      double pi = 3.14159;
      double vol10 = (4/3)*pi*pow(r10,3);
      double density = 1.65*pow(10,12);
      double mass10 = density*vol10;
      double K = 3531.5;
      float concLarge = (PM10count)*K*mass10;
      
      // next, PM2.5 count to mass concentration conversion
      double r25 = 0.44*pow(10,-6);
      double vol25 = (4/3)*pi*pow(r25,3);
      double mass25 = density*vol25;
      float concSmall = (PM25count)*K*mass25;
      

      Serial.print(PM10count);
      Serial.print(",");
      Serial.print(PM25count);
      Serial.print(",");
      Serial.print(concLarge);
      Serial.print(",");
      Serial.println(concSmall);
    
      durationP1 = 0;
      durationP2 = 0;
      starttime = millis();
      
 //Send SMS with GPRS Shield
  Serial.println("Sending dustduino SMS to Frontline server...");
  gprsSerial.print("AT+CMGF=1\r"); // Set the shield to SMS mode
  delay(100);

  gprsSerial.println("AT+CMGS = \"+NUMBER-PHONE\"");
  delay(100);
  gprsSerial.println("PM10count"); 
  delay(100);
  gprsSerial.println(PM10count); 
  delay(100);
  gprsSerial.println("PM10"); 
  delay(100);
  gprsSerial.println(concLarge);
  delay(100);
  gprsSerial.println("PM25count"); 
  delay(100);
  gprsSerial.println(PM25count);
  delay(100);
  gprsSerial.println("PM25");
  delay(100);
  gprsSerial.println(concSmall);
  delay(100);    
  gprsSerial.print((char)26);
  delay(100);
  gprsSerial.println();
  Serial.println("SMS Sent.");     
      
    } 

}

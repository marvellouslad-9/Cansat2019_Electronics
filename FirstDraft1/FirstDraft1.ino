#include <SoftwareSerial.h> //UART  : Including Library for Serial communication
#include <Wire.h>           //I2C   : Inclding Wire library for I2C communication
#include "RTClib.h"         //RTC   : Induding library for RTC DS3231
#include <TinyGPS++.h>      //GPS   : Library for GPS
#include "packet.h"         //PACKET:
#include <Adafruit_Sensor.h>//BMP   : 
#include <Adafruit_BMP280.h>//BMP   :
//==========================>>PACKET<<===============================================
packet p1(1);               //PACKET: This creates a variable packet
//==========================>>RTC<<===============================================
RTC_DS3231 rtc;             //RTC : Creating RTC Variable
uint32_t startTime = 0;     //RTC : StartTime will be initialize in setupRTC()

//==============================>>XBEE<<=============================================== We'll be creating seperate thread for sending data
SoftwareSerial xbee(5, 6);      //XBEE: RX, TX : D5 -to-> Tx of Xbee  and D6 -to-> Rx of Xbee
#define xBeeBaudRate 9600       //XBEE: data rate for the communication with other xbee

//===============================>>HALL<<===============================================
int dPin = 2;                    //HALL: D2 is connected to Digital pin of Hall
volatile unsigned long count = 0;//Hall: Counting the count of detection               
#define NUMBEROFMAGNETS 3        //Hall: Sensor: Number of magnets on fan 

//==============================>>GPS<<===============================================
#define GPSBaud 9600            //GPS : Baud Rate
TinyGPSPlus gps;                //GPS : Creating Tiny GPS object
SoftwareSerial ss(4, 3);        //GPS : Rx(D4) -> Tx of GPS   and   Tx(D3) -> Rx of GPS   

//==============================>>BMP<<===============================================
Adafruit_BMP280 bmp;            //BMP : Creating BMP object
float currentAltitute = 0;      //BMP : To save altitute of height

//===========================================>>SETUP and LOOP <<============================================================================= 
void setup() {
  Serial.begin(9600);       //For debugging purposes
  delay(3000);              // wait for console opening

  //Calling setting Functions
  setupRTC();               //RTC : Setup
  setupXBee();              //Xbee: Setup
  setupHall();              //HALL: Setup
  setupGPS();               //GPS : Setup
  setupBMP();               //BMP : Setup
}
void loop() {
  getMissionTime();         //RTC : Saving the mission time from RTC in packet p1
  getBMPReadings();         //BMP : Saving the height and pressure in packet variable

        
  delay(1000);              //No reason to add this. Just for visual debugging
}

//===========================================>> RTC Functions: Setting up and Initializing values <<============================================= 
void setupRTC(){
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  DateTime now = rtc.now();
  startTime = now.unixtime();
  p1.mission_time = 0;
}

void getMissionTime(){
  DateTime now = rtc.now();
  p1.mission_time = now.unixtime() - startTime;
}
//===========================================>> RTC : code finishes <<============================================= 
//===========================================>> Xbee Functions: Setting up and Initializing values <<============================================= 
void setupXBee(){
  xbee.begin(xBeeBaudRate); //xBee: set the data rate for the SoftwareSerial port
}

void XBeeSendPacket(){            //Most probably will be called by 1 Hz software timer
  xbee.print(p1.toString());
}

//===========================================>> XBEE : code finishes  <<============================================= 

//===========================================>> HALL Functions: Setting up and Initializing values <<============================================= 
void setupHall(){
  pinMode(dPin, INPUT);
  count = 0;                        //Initializing count to zero
}

/*
 * Called by Hardware Interrupt at digital pin 2 that is connected to Hall
 * Function: Increase global variable "count" of magnet by 1
 */
int raiseCount(){                             
  count++;
}


/*
 * RPM = number of rotations/time taken * 60
 *     = (count)/(currentTime - startTime) * 60 .   (count is count per magnet)
 */
void giveRPM(volatile unsigned long interruptStartTime){
  volatile unsigned long timeTaken = ((millis()/1000) - interruptStartTime); //unit: in seconds
  volatile unsigned long noOfRotations = count/NUMBEROFMAGNETS;    //Detection of "numberOfMagnets" of counts equals 1 rotation in that time
  p1.blade_spin_rate = (noOfRotations/timeTaken)*60;                       //Maths to get RPM
}
//===========================================>> HALL : code finishes  <<============================================= 

//===========================================>> GPS Functions: Setting up and Initializing values <<============================================= 
void setupGPS(){
  ss.begin(GPSBaud);
}

void getGPS(){
  getGPSAltitude();
  getGPSTime();
  getGPSLongitude();
  getGPSLatitute();
  getGPSNumSatelite();
}
/* Below 5 functions has to be completed from coe that worked*/
void getGPSNumSatelite(){
  
}
void getGPSLatitute(){
  
}
void getGPSLongitude(){
  
}
void getGPSTime(){
  
}
void getGPSAltitude(){
  
}
//===========================================>> GPS : code finishes  <<============================================= 

//===========================================>> BMP Functions: Setting up and Initializing values <<============================================= 
void setupBMP(){
  bmp.begin();
  if (!bmp.begin()) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
  }
  currentAltitute = findCurrentAltitute();
}
//Takes the mean of 2000 height at ground position and return it as current height
float findCurrentAltitute(){
  double sumCurrentHeight = 0;
  for(int i=0; i<2000; i++){
    sumCurrentHeight += bmp.readAltitude(1013.25);
  }
//  double currentAlt = sumCurrentHeight/2000; 
  return sumCurrentHeight/2000;
}
void getBMPReadings(){
  p1.pressure = bmp.readPressure();                           // in pascal
  p1.altitude = bmp.readAltitude(1013.25) - currentAltitute;  //unit in meter
  p1.temperature = bmp.readTemperature();                     //unit in celsius
}
//===========================================>> BMP : code finishes  <<============================================= 


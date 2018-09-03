/*
 * Title: Weatherstation Rev 1.1
 * Author: Vincent Piegsa
 * Date: 28.08.2018
 * Description: 
 *  -implemented auto-synchronization to the DS3231
 *  -implemented multiple-page layout to LCD
 *  -implemented data-formatting 
 *  -implemented timing for LCD and SD-Card
 */

//importing all necessary libraries
#include <Wire.h>
#include "RTClib.h"
#include <DHT.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>

//defininf pin and type of DHT11
#define DHTPIN 13
#define DHTTYPE DHT11

//initializing the DHT11
DHT dht(DHTPIN, DHTTYPE);

//initializing the DS3231 
RTC_DS3231 rtc;

//initializing the LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);  

//CS-Pin of the SD-Card Module
const int chipSelect = 6;

//variables for timing
int lcd_timer = 0;
int sd_timer = 0;
int lcd_page = 0;

//auto-initialization of the DS3231, if the Arduino has been off 
bool syncOnFirstStart = false; 

//function that inserts the measured data into an array
void formatArray(String* arr) {
  DateTime now = rtc.now();
  
  String time_ = String(now.hour(), DEC) + ":" + String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  String date_ = String(now.day(), DEC) + "." + String(now.month(), DEC) + "." + String(now.year(), DEC);

  String tmp = String(dht.readTemperature());
  String hdt = String(dht.readHumidity());

  arr[0] = date_;
  arr[1] = time_;
  arr[2] = tmp;
  arr[3] = hdt;
}

//function that displays the date and time on the LCD
void displayDatetime(String* data) {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print(data[0]);

  lcd.setCursor(0, 1);
  lcd.print(data[1]);;
}

//function that displays the temperature on the LCD
void displayTemperature(String* data) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  
  lcd.setCursor(0, 1);
  lcd.print(String(data[2]) + "°C");
}

//function that displays the humidity on the LCD
void displayHumidity(String* data) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Humidity: ");
  
  lcd.setCursor(0, 1);
  lcd.print(String(data[3]) + "%");
}

//function that formats into .csv format
String formatSD(String* data) {
  String dataString = "";
  
  for (int i = 0; i < 4; i++) {
    switch (i) {
      case 0:
        dataString += "\n" + data[i] + ";";
        break;
      case 1:
        dataString += data[i] + ";";
        break;
      case 2:
        dataString += data[i] + ";";
        break;
      case 3:
        dataString += data[i] + ";";
        break;
    }
  }

  return dataString;
}

//function that writes a string, containing the data, to the SD-Card
void writeSD(String dataString) {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
  }
  else {
    Serial.println("Error opening datalog.txt");
  }
}

//screen that shows when the Arduino is being restarted
void lcd_begin() {
  lcd.setCursor(0, 0);
  lcd.print("Weatherstation");
  
  lcd.setCursor(0, 1);
  lcd.print("Vincent Piegsa");
}

void setup() {
  //initialization of the serial connection
  Serial.begin(9600);

  //initialization of the DS3231
  if (! rtc.begin()) {
    Serial.println("initializing RTC failed");
  }

  //auto-initialization of the DS3231
  if (rtc.lostPower() || syncOnFirstStart) {
    Serial.println("Due to a powerloss, the RTC is being synchronized.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //initialization of the SD-Card Module
  if (!SD.begin(chipSelect)) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("Card initialization failed");
  }

  //initialization of the DHT11
  dht.begin();

  //initialization of the LCD
  lcd.begin(16, 2);
  
  //starting-screen 
  lcd_begin();
}

void loop() {
  //auto-increment of the timer-variables
  lcd_timer += 1;
  sd_timer += 1;

  //creating array with 4 String elements
  String data[4] = {};                    

  //inserting the values (date, time, temperature, humidity)
  formatArray(data);

  //if the timer-variable is 20000 (~1 minute)
  if (sd_timer == 20000) {
    //formatting the data
    String dataString = formatSD(data);  

    Serial.println("Writing to SD-Card...\n\t" + dataString);
    
    //writing the data to the SD-Card
    writeSD(dataString);                    

    //resetting the SD-Module-Timer
    sd_timer = 0;
  }

  //if the LCD-Timer is 1000 (~3 seconds)
  if (lcd_timer == 1000) {
    
    switch(lcd_page) {
      case 0:     
        //displaying date and time          
        displayDatetime(data);
        lcd_page += 1;
        break;
      case 1:
        //displaying the temperature
        displayTemperature(data);
        lcd_page += 1;
        break;
      case 2:
        //displaying the humidity
        displayHumidity(data);
        lcd_page = 0;
        break;
    }

    //resetting the LCD-Timer
    lcd_timer = 0;
  }
  
}

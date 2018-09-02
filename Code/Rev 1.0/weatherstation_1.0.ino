/*
 * Title: Weatherstation Rev 1.0
 * Author: Vincent Piegsa
 * Date: 26.08.2018
 * Description: 
 *  An Arduino-based weatherstation, including
 *  - DHT11 (Temperature & Humidity Sensor)
 *  - DS3231 (Clock)
 *  - SD-Module
 *  - LCD
 */

//importing all necessary libraries
#include <DS3231.h>             
#include <DHT.h>
#include <LiquidCrystal.h>
#include <SD.h>
#include <SPI.h>

//defining pin and type of DHT11 (temperature & humidity sensor)
#define DHTPIN 7                
#define DHTTYPE DHT11

//initializing DHT11
DHT dht(DHTPIN, DHTTYPE);

//initializing DS3231
DS3231 rtc(SDA, SCL);

//initializing LCD
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

//CS-Pin of the SD-Card Module
const int chipSelect = 6;

//function that splits the date and time (needed for the initialization of the DS3231)
String getSplitValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

//function that displays the humidity and temperature on the LCD
void display_lcd(String humidity, String temperature_) {
  lcd.setCursor(0, 0);
  lcd.print(rtc.getTimeStr());

  lcd.setCursor(0, 1);
  lcd.print(humidity);

  lcd.setCursor(8, 1);
  lcd.print(temperature_);
}

// function that prints the measured data via the serial connection
void serial(String humidity, String temperature_) {
  Serial.print(rtc.getDateStr());
  Serial.print(" -- ");
  Serial.print(rtc.getTimeStr());

  Serial.println();

  Serial.print(humidity);
  Serial.print(" % -- ");
  Serial.print(temperature_);
  Serial.print(" degree Celcius");

  Serial.println();
  delay(2000);
}

void setup() {
  //initializing the serial connection
  Serial.begin(9600);

  //initializing the DS3231
  rtc.begin();

  //initializing the DHT11
  dht.begin();

  //getting the data
  String date_ = String(__DATE__);

  //parsing the date to the splitting function
  int day = getSplitValue(date_, '.', 0).toInt();
  int month = getSplitValue(date_, '.', 1).toInt();
  int year_ = getSplitValue(date_, '.', 2).toInt();

  //getting the time
  String time_ = String(__TIME__);

  //parsing the time to the splitting function
  int hour_ = getSplitValue(time_, ':', 0).toInt();
  int second = getSplitValue(time_, ':', 1).toInt();
  int millisecond = getSplitValue(time_, ':', 2).toInt();

  //parsing the converted values to the DS3231
  rtc.setTime(hour_, second, millisecond);
  rtc.setDate(day, month, year_);

  //initializing the LCD
  lcd.begin(16, 2);

  //initializing the SD-Card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card initialization failed");
  }
}

void loop() {
  // variable for storing the data
  String dataString = "";

  //displaying the temperature and humidity
  display_lcd(String(dht.readHumidity()) + "%", String(dht.readTemperature()) + " C");

  //adding temperature and humidity to dataString
  dataString += "Temperature: " + String((dht.readHumidity() + rtc.getTemp())/2) + "%; ";
  dataString += "Humidity: " + String(dht.readTemperature()) + " C; ";

  //adding date and time to dataString
  dataString += String(rtc.getDateStr()) + " -- ";
  dataString += String(rtc.getTimeStr()) + ";";

  //opening datalog.txt on the SD-Card
  File dataFile = SD.open("datalog.txt", FILE_WRITE);

  //writing to datalog.txt if exists/connection is available
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();

    Serial.println(dataString);
  }
  else {
    Serial.println("error opening datalog.txt");
  }

  delay(1000);
}

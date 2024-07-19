#include <LiquidCrystal_I2C.h>      // import LCD I2C Library
#include <OneWire.h>                // DS18B20 library 
#include <DallasTemperature.h>      // DS18B20 library
#include "DHT.h"                    // DHT library
#define DHTPIN 13
#define DHTTYPE DHT11

void(* resetFunc) (void) = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address 0x27, 16 column and 2 rows
DHT dht(DHTPIN, DHTTYPE);

// Set the Arduino pin output to correlating pin number
const int RELAY_PIN = 3;   // Arduino pin connected to Relay Pin
const int SENSOR_PIN = 13; // Arduino pin connected to DS18B20 sensor's DQ pin

// variable to store previous time
unsigned long relayPreviousMillis = 0; 
unsigned long sensorPreviousMillis = 0;
unsigned long resetPreviousMillis = 0;

// intervals for millis() ; convert time to milliseconds
const long onInterval = 60000;        // turn on for # minutes
const long offInterval = 300000;      // turn off for # minutes
const long sensorInterval = 2000; // read temp for # seconds
const long resetInterval = 720000;  // reset for # minute


bool relayState = true;
bool sensorState = false;

void setup() {
  // Setup Serial for debugging
  Serial.begin(9600); // int Serial

  // Set the pin output
  pinMode(RELAY_PIN, OUTPUT);

  // Initualise the LCD and Sensor
  lcd.init();         // initialize the lcd
  lcd.backlight();    // open the backlight 
  dht.begin();     // initialize the sensor

  // Print initializing Program
  lcd.setCursor(0, 0);       // start to print at the first row
  lcd.print("Initializing"); 
  lcd.setCursor(0, 1);       // start to print at the first row
  lcd.print("Program");
  Serial.println("Initializing Program"); // Serial Debugging
  delay(2000);
  lcd.clear(); // Clear Display
}

void loop() {
  float tempC = dht.readTemperature(); // read temperature
  float humi  = dht.readHumidity();    // read humidity

  unsigned long currentMillis = millis();

  // Calculate remaining time for the current state (on or off)
  unsigned long remainingTime;
  if (relayState) {
    remainingTime = onInterval - (currentMillis - relayPreviousMillis);
  } else {
    remainingTime = offInterval - (currentMillis - relayPreviousMillis);
  }
  
  // Print if Pump is ON or OFF
  lcd.setCursor(0, 0);
  if (relayState) {
    lcd.print("Pump ON");
  } else {
    lcd.print("Pump OFF");
  }

  // Print the remaining time
  lcd.setCursor(10, 0);
  lcd.print(formatTime(remainingTime));

  /* Relay */
  // Turn ON Relay 
  if (!relayState && currentMillis - relayPreviousMillis >= offInterval) {
    // If the relay is off and the off interval has elapsed, turn it on
    relayState = true;
    digitalWrite(RELAY_PIN, LOW);        // Turn on the relay
    relayPreviousMillis = currentMillis; // Save the last time the relay was toggled
    // Print status on LCD and Serial 
    lcd.setCursor(0,0);           // clear line 0
    lcd.print("                ");
    Serial.println("Pump ON");    // For debugging
  }

  // Turn OFF Relay
  else if (relayState && currentMillis - relayPreviousMillis >= onInterval) {
    // If the relay is on and the on interval has elapsed, turn it off
    relayState = false;
    digitalWrite(RELAY_PIN, HIGH);
    relayPreviousMillis = currentMillis; // Save the last time the relay was toggled

    // Print status on LCD and Serial 
    lcd.setCursor(0,0);           // clear line 0
    lcd.print("                ");
    Serial.println("Pump OFF");   // For debugging
  }
  /* Temperature Sensor */
  if (!sensorState && currentMillis - sensorPreviousMillis >= sensorInterval) {
    sensorState = true;
    // Print temperature sensor data on LCD
    lcd.setCursor(0,1);           // clear line 1
    lcd.print("                ");
  // check if any reads failed
  if (isnan(tempC)) {
    lcd.setCursor(0, 1);
    lcd.print("Failed");
    } else {
    lcd.setCursor(0, 1);  // start to print at the first row
    lcd.print("Temp: ");
    lcd.print(tempC);     // print the temperature
    lcd.print((char)223); // print ° character
    lcd.print("C");
    Serial.print("Temperature: ");   // For debugging
    Serial.print(humi);
    Serial.println("°C");
    sensorPreviousMillis = currentMillis; // Save the last time temperature sensor was toggled
    }
  }
  /* Humid Sensor */
  else if (sensorState &&currentMillis - sensorPreviousMillis >= sensorInterval) {
    sensorState = false;
    // Print humid sensor data on LCD
    lcd.setCursor(0,1);           // clear line 1
    lcd.print("                ");
  // check if any reads failed
  if (isnan(humi)) {
    lcd.setCursor(0, 1);
    lcd.print("Failed");
    } else {
    lcd.setCursor(0, 1);  // start to print at the second row
    lcd.print("Humi: ");
    lcd.print(humi);      // print the humidity
    lcd.print("%");
    Serial.print("Humidity: ");   // For debugging
    Serial.print(humi);
    Serial.println("%");
    sensorPreviousMillis = currentMillis; // Save the last time temperature sensor was toggled
    }
  }

  // reset # minute
  if (currentMillis - resetPreviousMillis >= resetInterval) {
    resetFunc();
    resetPreviousMillis = currentMillis;
  }

}

// Function to format milliseconds into MM:SS format
String formatTime(unsigned long milliseconds) {
  unsigned long seconds = milliseconds / 1000;
  unsigned long minutes = seconds / 60;

  seconds %= 60;
  minutes %= 60;

  char buffer[9]; // Format string as MM:SS
  sprintf(buffer, "%02lu:%02lu", minutes, seconds);
  return String(buffer);
}




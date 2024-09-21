#define BLYNK_TEMPLATE_ID "TMPL6eLEWWY0G"  // Make sure this is your correct Template ID
#define BLYNK_TEMPLATE_NAME "Smart Agriculture System"  // Make sure this is your correct Template Name
#define BLYNK_AUTH_TOKEN "TxNcjux7SM66CiFmmTUwvUccW-Nf0dhC"  // Blynk Auth Token

#define BLYNK_PRINT Serial  // Debug prints to serial monitor
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>  // Correct Blynk library for ESP8266
#include <DHT.h>  // DHT sensor library
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Library for I2C LCD

#define DHTPIN D2  // Pin for DHT22 sensor
#define DHTTYPE DHT22  // DHT 22 (AM2302), AM2321

#define SOIL_MOISTURE_PIN A0  // Analog pin for soil moisture sensor
#define THRESHOLD_MOISTURE 10  // Threshold for soil moisture percentage
#define PUMP_PIN D1  // Pin for controlling the pump
#define PUMP_SWITCH V6  // Virtual pin for manual pump control
#define BUZZER_PIN D3  // Pin for buzzer

const char auth[] = BLYNK_AUTH_TOKEN;  // Blynk Auth Token
const char ssid[] = "okay";  // Your WiFi SSID
const char pass[] = "12345678";  // Your WiFi password

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

BlynkTimer timer;
bool isPumpOn = false;  // Track pump status

const char* msg_soil_moisture = "Soil Moisture: ";
const char* msg_soil_moisture_percentage = "Soil Moisture Percentage: ";
const char* msg_humidity = "Humidity: ";
const char* msg_temperature = "Temperature: ";
const char* msg_failed_dht = "Failed to read from DHT sensor!";
const char* msg_pump_on_auto = "Pump turned on due to low soil moisture!";
const char* msg_pump_on_manual = "Pump manually turned ON";
const char* msg_pump_off_manual = "Pump manually turned OFF";

// Function to read sensor data and control the pump
void sendSensorData() {
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  Serial.print(msg_soil_moisture);
  Serial.println(soilMoisture);

  // Map sensor value to percentage (tune values based on your sensor)
  int soilMoisturePercentage = map(soilMoisture, 400, 1023, 100, 0);
  soilMoisturePercentage = constrain(soilMoisturePercentage, 0, 100);  // Ensure percentage is between 0 and 100

  Serial.print(msg_soil_moisture_percentage);
  Serial.println(soilMoisturePercentage);

  // Send data to Blynk app
  Blynk.virtualWrite(V5, soilMoisturePercentage);

  // Automatically control the pump based on moisture level if not manually turned on
  if (!isPumpOn && soilMoisturePercentage < THRESHOLD_MOISTURE) {
    digitalWrite(PUMP_PIN, HIGH);  // Turn on pump
    digitalWrite(BUZZER_PIN, HIGH);  // Turn on buzzer
    delay(500);  // Buzzer on for 500ms
    digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
    Serial.println(msg_pump_on_auto);
    Blynk.logEvent("moisture_alert", "Soil moisture is below the threshold! Pump turned ON.");
  } else if (!isPumpOn) {
    digitalWrite(PUMP_PIN, LOW);  // Turn off pump
  }

  // Read DHT22 sensor data
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(msg_failed_dht);
    h = -128.0;  // Invalid value
    t = -128.0;  // Invalid value
  } else {
    Serial.print(msg_humidity);
    Serial.print(h);
    Serial.print(" %\t");
    Serial.print(msg_temperature);
    Serial.print(t);
    Serial.println(" *C");
  }

  // Send DHT22 data to Blynk app
  Blynk.virtualWrite(V7, h);  // Humidity
  Blynk.virtualWrite(V8, t);  // Temperature

  // Display data on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.print(soilMoisturePercentage);
  lcd.print("%");
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print("C Hum: ");
  lcd.print(h);
  lcd.print("%");
}

// Manual control of the pump via Blynk app
BLYNK_WRITE(PUMP_SWITCH) {
  isPumpOn = param.asInt();
  if (isPumpOn) {
    digitalWrite(PUMP_PIN, HIGH);  // Turn on pump manually
    digitalWrite(BUZZER_PIN, HIGH);  // Turn on buzzer
    delay(500);  // Buzzer on for 500ms
    digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
    Serial.println(msg_pump_on_manual);
  } else {
    digitalWrite(PUMP_PIN, LOW);  // Turn off pump manually
    digitalWrite(BUZZER_PIN, HIGH);  // Turn on buzzer
    delay(500);  // Buzzer on for 500ms
    digitalWrite(BUZZER_PIN, LOW);  // Turn off buzzer
    Serial.println(msg_pump_off_manual);
  }
}

void setup() {
  Serial.begin(115200);  // Use higher baud rate for better serial output
  pinMode(PUMP_PIN, OUTPUT);  // Set pump pin as an output
  pinMode(BUZZER_PIN, OUTPUT);  // Set buzzer pin as an output

  Blynk.begin(auth, ssid, pass);  // Connect to Blynk

  dht.begin();  // Initialize DHT sensor
  lcd.begin();  // Initialize LCD

  // Print static labels to LCD
  lcd.setCursor(0, 0);
  lcd.print("Moisture: ");
  lcd.setCursor(0, 1);
  lcd.print("Temp:     C Hum:     %");

  timer.setInterval(3000L, sendSensorData);  // Set sensor check every 3 seconds

  Blynk.syncVirtual(PUMP_SWITCH);  // Sync the manual pump control state
}

void loop() {
  Blynk.run();  // Run Blynk
  timer.run();  // Run timer for periodic sensor checks
}

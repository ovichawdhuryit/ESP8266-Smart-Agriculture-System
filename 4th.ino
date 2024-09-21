#define BLYNK_TEMPLATE_ID "TMPL6eLEWWY0G"     // Make sure this is your correct Template ID
#define BLYNK_TEMPLATE_NAME "Smart Agriculture System"  // Make sure this is your correct Template Name
#define BLYNK_AUTH_TOKEN "TxNcjux7SM66CiFmmTUwvUccW-Nf0dhC"  // Blynk Auth Token

#define BLYNK_PRINT Serial  // Debug prints to serial monitor
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>  // Correct Blynk library for ESP8266

#define SOIL_MOISTURE_PIN A0   // Analog pin for soil moisture sensor
#define THRESHOLD_MOISTURE 10  // Threshold for soil moisture percentage
#define PUMP_PIN D1            // Pin for controlling the pump
#define PUMP_SWITCH V6         // Virtual pin for manual pump control

char auth[] = BLYNK_AUTH_TOKEN;  // Blynk Auth Token
char ssid[] = "okay";            // Your WiFi SSID
char pass[] = "12345678";        // Your WiFi password

BlynkTimer timer;
bool isPumpOn = false;           // Track pump status

// Function to read sensor data and control the pump
void sendSensorData() {
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);

  // Map sensor value to percentage (tune values based on your sensor)
  int soilMoisturePercentage = map(soilMoisture, 400, 1023, 100, 0);
  soilMoisturePercentage = constrain(soilMoisturePercentage, 0, 100);  // Ensure percentage is between 0 and 100

  Serial.print("Soil Moisture Percentage: ");
  Serial.println(soilMoisturePercentage);

  // Send data to Blynk app
  Blynk.virtualWrite(V5, soilMoisturePercentage);

  // Automatically control the pump based on moisture level if not manually turned on
  if (!isPumpOn && soilMoisturePercentage < THRESHOLD_MOISTURE) {
    digitalWrite(PUMP_PIN, HIGH);  // Turn on pump
    Serial.println("Pump turned on due to low soil moisture!");
    Blynk.logEvent("moisture_alert", "Soil moisture is below the threshold! Pump turned ON.");
  } else if (!isPumpOn) {
    digitalWrite(PUMP_PIN, LOW);   // Turn off pump
  }
}

// Manual control of the pump via Blynk app
BLYNK_WRITE(PUMP_SWITCH) {
  isPumpOn = param.asInt();
  if (isPumpOn) {
    digitalWrite(PUMP_PIN, HIGH);  // Turn on pump manually
    Serial.println("Pump manually turned ON");
  } else {
    digitalWrite(PUMP_PIN, LOW);   // Turn off pump manually
    Serial.println("Pump manually turned OFF");
  }
}

void setup() {
  Serial.begin(115200);  // Use higher baud rate for better serial output
  pinMode(PUMP_PIN, OUTPUT); // Set pump pin as an output

  Blynk.begin(auth, ssid, pass);  // Connect to Blynk

  timer.setInterval(3000L, sendSensorData);  // Set sensor check every 3 seconds

  Blynk.syncVirtual(PUMP_SWITCH);  // Sync the manual pump control state
}

void loop() {
  Blynk.run();    // Run Blynk
  timer.run();    // Run timer for periodic sensor checks
}

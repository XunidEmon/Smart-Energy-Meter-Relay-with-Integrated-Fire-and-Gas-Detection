/*
 * Smart Energy Meter & Relay with Fire & Gas Detection
 * ESP32-based system with OLED, sensors, relay, and Google Sheets
 */

// ==================== LIBRARIES ====================
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HTTPClient.h>

// ==================== OLED SETUP ====================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==================== PIN DEFINITIONS ====================
#define VOLTAGE_PIN   34
#define CURRENT_PIN   35
#define GAS_PIN       32
#define FLAME_PIN     33
#define RELAY_PIN     12
#define BUZZER_PIN    13

// ==================== THRESHOLDS ====================
#define GAS_THRESHOLD  1000   // Adjust based on sensor
#define VOLTAGE_MAX    25.0   // Voltage sensor max
#define CURRENT_MAX    5.0    // Current sensor max (ACS712-5A)

// ==================== Wi-Fi SETUP ====================
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String googleScriptURL = "YOUR_GOOGLE_APPS_SCRIPT_URL";

// ==================== VARIABLES ====================
float voltage = 0;
float current = 0;
float power = 0;
int gasValue = 0;
int flameValue = 0;
bool gasDetected = false;
bool flameDetected = false;

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);
  
  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("OLED allocation failed");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.display();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  
  // Initial display
  displayMessage("System Ready!");
  delay(2000);
}

// ==================== LOOP ====================
void loop() {
  // 1. Read Sensors
  readSensors();
  
  // 2. Check Hazards
  checkHazards();
  
  // 3. Update OLED
  updateDisplay();
  
  // 4. Send to Google Sheets
  sendToGoogleSheets();
  
  delay(2000);  // Update every 2 seconds
}

// ==================== FUNCTIONS ====================

void readSensors() {
  // Read voltage (0-25V)
  int voltageADC = analogRead(VOLTAGE_PIN);
  voltage = (voltageADC / 4095.0) * VOLTAGE_MAX * 2.5;  // Adjust factor
  
  // Read current (0-5A)
  int currentADC = analogRead(CURRENT_PIN);
  float currentOffset = 2.5;  // ACS712 offset at 0A
  current = ((currentADC / 4095.0 * 5.0) - currentOffset) / 0.185;  // 185mV/A
  
  // Read gas sensor
  gasValue = analogRead(GAS_PIN);
  
  // Read flame sensor (digital)
  flameValue = digitalRead(FLAME_PIN);
  
  // Calculate power
  power = voltage * current;
  
  // Print debug
  Serial.print("V: "); Serial.print(voltage);
  Serial.print(" | I: "); Serial.print(current);
  Serial.print(" | P: "); Serial.print(power);
  Serial.print(" | Gas: "); Serial.print(gasValue);
  Serial.print(" | Flame: "); Serial.println(flameValue);
}

void checkHazards() {
  // Gas detection
  if(gasValue > GAS_THRESHOLD) {
    gasDetected = true;
    digitalWrite(RELAY_PIN, LOW);  // Cut power
    alarmSound(1);  // Continuous fast beep
  } else {
    gasDetected = false;
  }
  
  // Fire detection (flame sensor outputs LOW when fire detected)
  if(flameValue == LOW) {
    flameDetected = true;
    digitalWrite(RELAY_PIN, LOW);  // Cut power
    alarmSound(2);  // 3 short beeps
  } else {
    flameDetected = false;
  }
  
  // Normal state
  if(!gasDetected && !flameDetected) {
    digitalWrite(RELAY_PIN, HIGH);  // Power ON
    noTone(BUZZER_PIN);  // Buzzer OFF
  }
}

void alarmSound(int type) {
  if(type == 1) {  // Gas leak: Continuous fast beep
    tone(BUZZER_PIN, 2000, 100);
    delay(150);
    tone(BUZZER_PIN, 2000, 100);
    delay(150);
    tone(BUZZER_PIN, 2000, 100);
    delay(150);
    tone(BUZZER_PIN, 2000, 100);
    delay(150);
  }
  else if(type == 2) {  // Fire: 3 short beeps
    tone(BUZZER_PIN, 3000, 200);
    delay(300);
    tone(BUZZER_PIN, 3000, 200);
    delay(300);
    tone(BUZZER_PIN, 3000, 200);
    delay(300);
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.println("=== ENERGY METER ===");
  display.print("V: "); display.print(voltage, 2); display.println(" V");
  display.print("I: "); display.print(current, 2); display.println(" A");
  display.print("P: "); display.print(power, 2); display.println(" W");
  
  // Gas status
  display.print("Gas: ");
  if(gasDetected) {
    display.println("DANGER!");
  } else {
    display.println("Safe");
  }
  
  // Fire status
  display.print("Fire: ");
  if(flameDetected) {
    display.println("DETECTED!");
  } else {
    display.println("No Fire");
  }
  
  display.display();
}

void sendToGoogleSheets() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = googleScriptURL + 
                 "?voltage=" + String(voltage, 2) +
                 "&current=" + String(current, 2) +
                 "&power=" + String(power, 2) +
                 "&gas=" + String(gasValue) +
                 "&flame=" + String(flameDetected);
    
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    
    int httpCode = http.GET();
    if(httpCode > 0) {
      Serial.println("Data sent to Google Sheets!");
    } else {
      Serial.println("Failed to send data");
    }
    http.end();
  }
}

void displayMessage(String msg) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(msg);
  display.display();
}
#  Smart Energy Meter & Relay with Fire & Gas Detection
##  Overview

This project combines **energy monitoring** and **safety features** into one affordable system. It helps users:

-  **Track real-time power consumption** (Voltage, Current, Power)
-  **Detect gas leaks** using MQ-2 sensor
-  **Detect fire** using flame sensor
-  **Automatically cut off power** during emergencies via relay
-  **Sound different alarms** for gas (continuous beep) and fire (3 short beeps)
-  **Display data** on OLED screen
-  **Send data to Google Sheets** for remote monitoring via Wi-Fi

---

##  Features

| Feature | Description |
|---------|-------------|
|  **Energy Monitoring** | Measures voltage, current, and power consumption |
|  **Gas Detection** | MQ-2 sensor detects LPG, methane, propane |
|  **Fire Detection** | Flame sensor detects fire using infrared |
|  **Auto Power Cut-off** | Relay cuts power during gas/fire emergencies |
|  **Dual Alarm System** | Different sounds for gas (continuous) and fire (3 beeps) |
|  **OLED Display** | Shows real-time data locally |
|  **Google Sheets Logging** | Remote monitoring via Wi-Fi |
|  **IoT Ready** | Can be extended with mobile app |

---

##  Components Required

| Component | Quantity | Purpose |
|-----------|----------|---------|
| **ESP32 Microcontroller** | 1 | Main controller with Wi-Fi |
| **Voltage Detection Sensor (25V)** | 1 | Measures AC voltage |
| **ACS712-5A Current Sensor** | 1 | Measures current up to 5A |
| **MQ-2 Gas Sensor** | 1 | Detects LPG, methane, propane |
| **Flame Sensor** | 1 | Detects fire |
| **Relay Module (5V)** | 1 | Automatic power switch |
| **OLED Display (0.96" I2C)** | 1 | Shows live data |
| **Buzzer (5V)** | 1 | Alarm system |
| **5V USB Power Supply** | 1 | Powers all components |
| **Jumper Wires** | Several | Circuit connections |
| **Breadboard** | 1 | Circuit assembly |

---

##  How It Works

### System Flow Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                    SMART ENERGY METER SYSTEM                   │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐    │
│  │   Voltage    │    │   Current    │    │    MQ-2      │    │
│  │   Sensor     │    │   Sensor     │    │  Gas Sensor  │    │
│  │   (25V)      │    │  (ACS712)    │    │              │    │
│  └──────┬───────┘    └──────┬───────┘    └──────┬───────┘    │
│         │                   │                   │             │
│         └───────────────────┼───────────────────┘             │
│                             │                                 │
│                      ┌──────▼───────┐                         │
│                      │   ESP32      │                         │
│                      │ Microcontrol │                         │
│                      └──────┬───────┘                         │
│         ┌───────────────────┼───────────────────┐             │
│         │                   │                   │             │
│  ┌──────▼───────┐    ┌──────▼───────┐    ┌──────▼───────┐    │
│  │   OLED       │    │   Relay      │    │   Buzzer     │    │
│  │   Display    │    │   Module     │    │              │    │
│  └──────────────┘    └──────┬───────┘    └──────────────┘    │
│                             │                                 │
│                      ┌──────▼───────┐                         │
│                      │  Power Load  │                         │
│                      │  (Appliance) │                         │
│                      └──────────────┘                         │
│                                                                 │
│  ☁️ Wi-Fi → Google Sheets (Remote Monitoring)                  │
└─────────────────────────────────────────────────────────────────┘
```

### Step-by-Step Working

1. **Power On** → ESP32 starts and initializes all components

2. **Energy Monitoring** → ESP32 reads voltage and current sensors to calculate:
   - Voltage (V)
   - Current (A)
   - Power (W)

3. **Hazard Detection** → ESP32 reads:
   - MQ-2 Gas Sensor (analog output)
   - Flame Sensor (digital output)

4. **Decision Making**:
   -  **Normal**: Relay stays ON, all LEDs normal
   -  **Gas Leak**: Relay OFF + Continuous fast beeping
   -  **Fire**: Relay OFF + 3 short beeps (repeating)

5. **Display Data** → OLED shows:
   - Voltage, Current, Power
   - Gas status (Safe/Warning/Danger)
   - Fire status (No Fire/Fire Detected)

6. **Remote Monitoring** → Sends data to Google Sheets via Wi-Fi

---

##  Circuit Diagram

### Pin Connections

| ESP32 Pin | Component | Purpose |
|-----------|-----------|---------|
| 5V | All Components | Power supply |
| GND | All Components | Ground |
| GPIO 21 (SDA) | OLED Display | I2C Data |
| GPIO 22 (SCL) | OLED Display | I2C Clock |
| GPIO 32 | MQ-2 Sensor | Analog gas reading |
| GPIO 33 | Flame Sensor | Digital fire detection |
| GPIO 34 | Voltage Sensor | Analog voltage reading |
| GPIO 35 | ACS712 Current | Analog current reading |
| GPIO 12 | Relay Module | Control power switch |
| GPIO 13 | Buzzer | Alarm output |

### Circuit Diagram

```
                    ESP32 DEV BOARD
                ┌─────────────────────┐
                │                     │
        5V ─────┤ 5V                  │
        GND ────┤ GND                 │
                │                     │
   OLED SDA ────┤ GPIO 21 (SDA)      │
   OLED SCL ────┤ GPIO 22 (SCL)      │
                │                     │
   MQ-2 OUT ────┤ GPIO 32 (ADC)      │
   Flame OUT ───┤ GPIO 33 (DIGITAL)  │
   Voltage OUT ─┤ GPIO 34 (ADC)      │
   ACS712 OUT ──┤ GPIO 35 (ADC)      │
                │                     │
   Relay IN ────┤ GPIO 12             │
   Buzzer + ────┤ GPIO 13             │
                │                     │
                └─────────────────────┘
```

---

##  Arduino Code

### Main Code: `smart_energy_meter.ino`

```cpp
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
```

---

## 📡 Google Sheets Setup

### Step 1: Create Google Apps Script

1. Open Google Sheets
2. Go to **Extensions → Apps Script**
3. Paste this code:

```javascript
function doGet(e) {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getActiveSheet();
  
  var voltage = e.parameter.voltage;
  var current = e.parameter.current;
  var power = e.parameter.power;
  var gas = e.parameter.gas;
  var flame = e.parameter.flame;
  
  sheet.appendRow([
    new Date(),
    voltage,
    current,
    power,
    gas,
    flame
  ]);
  
  return ContentService.createTextOutput("Success");
}
```

4. Deploy as Web App
5. Copy the URL → Use in Arduino code

### Step 2: Google Sheets Headers

| Timestamp | Voltage (V) | Current (A) | Power (W) | Gas Value | Fire Detected |
|-----------|-------------|-------------|-----------|-----------|---------------|
| 2024-01-15 10:00 | 220.5 | 2.3 | 506.5 | 450 | FALSE |

---

## 📊 Serial Monitor Output

```
=== SMART ENERGY METER STARTED ===
Wi-Fi Connected!
IP: 192.168.1.100

V: 220.45 | I: 2.35 | P: 518.05 | Gas: 450 | Flame: 1
V: 220.50 | I: 2.34 | P: 515.97 | Gas: 460 | Flame: 1
V: 220.48 | I: 2.36 | P: 520.33 | Gas: 450 | Flame: 1
⚠️ GAS LEAK DETECTED! Relay OFF!
V: 220.50 | I: 0.00 | P: 0.00 | Gas: 1200 | Flame: 1
⚠️ GAS LEAK DETECTED! Relay OFF!
🔥 FIRE DETECTED! Relay OFF!
```

---


```

---

## ✅ Advantages

| Advantage | Description |
|-----------|-------------|
| **All-in-One** | Energy meter + gas detector + fire alarm |
| **Automatic Safety** | Cuts power during emergencies |
| **Real-Time Data** | OLED display shows live readings |
| **Remote Monitoring** | Google Sheets access from anywhere |
| **Cost-Effective** | Cheaper than commercial systems |
| **Easy to Install** | No special training needed |
| **Saves Money** | Track usage to reduce bills |
| **Educational** | Great for learning IoT |

---

##  Limitations

| Limitation | Description |
|------------|-------------|
| Not for large industries | Designed for homes and small shops |
| Sensor accuracy | Not as accurate as professional sensors |
| Wi-Fi required | Needed for remote monitoring |
| Prototype stage | More development for commercial use |
| Buzzer volume | May not be heard in noisy areas |

---

##  Applications

| Place | Use |
|-------|-----|
|  **Homes** | Monitor power usage and stay safe |
|  **Small Shops** | Reduce costs and ensure safety |
|  **Kitchens** | Detect gas leaks and prevent accidents |
|  **Labs** | Protect equipment from fire |
|  **Schools** | Practical learning tool |
|  **Elderly Care** | Remote safety monitoring |
|  **Restaurants** | Safe and cost-effective management |

---

##  Future Scope

| Feature | Description |
|---------|-------------|
|  **Mobile App** | Real-time data and notifications |
|  **Voice Assistant** | Alexa/Google Home integration |
|  **Professional Product** | Custom PCB and certifications |
|  **Solar Integration** | Monitor grid and solar energy |
|  **Data Analytics** | Energy usage trends and suggestions |
|  **Multi-Device** | Monitor multiple rooms or floors |
|  **More Sensors** | Temperature, humidity, CO, smoke |

---

##  Team Members

| Name | ID | Contribution |
|------|-----|--------------|
| **Md Asadus Zaman Emon** | 23208052 | Circuit design, ESP32 programming, Google Sheets integration |
| **Md Ferdous Hossain** | 23208008 | Documentation, Testing, Report writing |
| **Shafa Bintea Rahman** | 23208053 | Sensor testing, Assembly, Presentation |
| **Nafisa Anjum Zeba** | 23208002 | Documentation, Simulation, Result analysis |

---

##  References

| Reference | Description |
|-----------|-------------|
| [1] | Alam et al., "An analytical approach to real-time cloud services on IoT-based applications for smart city planning," *Int. J. Grid Util. Comput.*, 2021 |
| [2] | Khan et al., "IoT-based applications in power and energy sector," *Int. J. Grid Util. Comput.*, 2021 |
| [3] | taifur20, "smart_home_hub," GitHub, 2023 |
| [4] | Star Business Report, "Short circuits still cause one in three fires despite warnings," *The Daily Star*, 2026 |

---

##  Quick Start Guide

```bash

# Open Arduino IDE
# Install required libraries:
# - Adafruit GFX Library
# - Adafruit SSD1306
# - WiFi
# - HTTPClient

# Upload code to ESP32
# Open Serial Monitor (115200 baud)
# Connect components according to circuit diagram
# Power ON the system
```

---

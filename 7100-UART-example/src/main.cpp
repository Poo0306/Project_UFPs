#include <Arduino.h>
#include <WiFi.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <time.h>
#include <map>
#include <string>

// Function prototypes
void updateLED(bool state);
String getFormattedDateTime(bool isDate);

// Pin and constant definitions
constexpr int RXPIN = 16;
constexpr int TXPIN = 17;
constexpr int LED_PIN = 2;
constexpr unsigned long SENSOR_WARM_UP_TIME = 2000;
constexpr unsigned long LED_BLINK_DURATION = 750;

// Network credentials
const char* const WIFI_SSID = "ENGIOT";
const char* const WIFI_PASSWORD = "coeai123";

// Firebase configuration
const char* const API_KEY = "AIzaSyDc8uubCOB8n7pCyMBcrxA1deAYaofppNw";
const char* const DATABASE_URL = "https://ufps-39155-default-rtdb.asia-southeast1.firebasedatabase.app/";
const char* const USER_EMAIL = "poo.t2546@gmail.com";
const char* const USER_PASSWORD = "poo03062546";

// Time configuration
const char* const NTP_SERVER = "pool.ntp.org";
constexpr long GMT_OFFSET_SEC = 25200;
constexpr int DAYLIGHT_OFFSET_SEC = 0;

struct PMData {
    float pm01;
    float pm03;
    float pm05;
    float pm10;
    float pm25;
    float pm50;
    float pm100;
    bool isValid;

    PMData() : pm01(0), pm03(0), pm05(0), pm10(0), pm25(0), pm50(0), pm100(0), isValid(false) {}
};

// Global objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
PMData sensorData;
String databasePath;

void setupWiFi() {
    Serial.print("Connecting to Wi-Fi");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }
    
    Serial.printf("\nConnected with IP: %s\n", WiFi.localIP().toString().c_str());
}

void setupFirebase() {
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    
    Serial.print("Signing in user");
    while (auth.token.uid == "") {
        delay(1000);
        Serial.print(".");
    }
    
    Serial.printf("\nSign in successful! UID: %s\n", auth.token.uid.c_str());
    databasePath = "/PieraData/" + String(auth.token.uid.c_str());
}

void setupTime() {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    
    Serial.print("Syncing NTP time");
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
    }
    Serial.println();
}

void setupSensor() {
    Serial.println("Warming up sensor...");
    delay(SENSOR_WARM_UP_TIME);
    Serial2.write("$Wazure=0\r\n");
    delay(100);
}

bool parseSensorData(const String& rawData, PMData& data) {
    std::map<String, float> dataMap;
    
    // Split the data string into key-value pairs
    int startPos = 0;
    bool hasPMValues = false;
    
    while (startPos < rawData.length()) {
        int commaPos = rawData.indexOf(',', startPos);
        if (commaPos == -1) break;
        
        String key = rawData.substring(startPos, commaPos);
        startPos = commaPos + 1;
        
        commaPos = rawData.indexOf(',', startPos);
        if (commaPos == -1) commaPos = rawData.length();
        
        String valueStr = rawData.substring(startPos, commaPos);
        float value = valueStr.toFloat();
        
        // Store only PM values, not PC values
        if (key.startsWith("PM")) {
            dataMap[key] = value;
            hasPMValues = true;
        }
        
        startPos = commaPos + 1;
    }
    
    // Only update if we found PM values
    if (hasPMValues) {
        data.pm01 = dataMap.count("PM0.1") ? dataMap["PM0.1"] : data.pm01;
        data.pm03 = dataMap.count("PM0.3") ? dataMap["PM0.3"] : data.pm03;
        data.pm05 = dataMap.count("PM0.5") ? dataMap["PM0.5"] : data.pm05;
        data.pm10 = dataMap.count("PM1.0") ? dataMap["PM1.0"] : data.pm10;
        data.pm25 = dataMap.count("PM2.5") ? dataMap["PM2.5"] : data.pm25;
        data.pm50 = dataMap.count("PM5.0") ? dataMap["PM5.0"] : data.pm50;
        data.pm100 = dataMap.count("PM10") ? dataMap["PM10"] : data.pm100;
        data.isValid = true;
        return true;
    }
    
    return false;
}

bool readSensorData() {
    updateLED(true);
    
    String rawData = Serial2.readStringUntil('\n');
    if (rawData.isEmpty()) return false;
    
    Serial.println("Received Data: " + rawData);
    
    if (parseSensorData(rawData, sensorData)) {
        Serial.println("Parsed Values:");
        Serial.printf("PM0.1: %.5f\nPM0.3: %.5f\nPM0.5: %.5f\nPM1.0: %.5f\n"
                     "PM2.5: %.5f\nPM5.0: %.5f\nPM10: %.5f\n",
                     sensorData.pm01, sensorData.pm03, sensorData.pm05,
                     sensorData.pm10, sensorData.pm25, sensorData.pm50,
                     sensorData.pm100);
        return true;
    }
    
    return false;
}

void sendToFirebase() {
    if (!sensorData.isValid) return;
    
    FirebaseJson json;
    json.set("PM01", sensorData.pm01);
    json.set("PM03", sensorData.pm03);
    json.set("PM05", sensorData.pm05);
    json.set("PM10", sensorData.pm10);
    json.set("PM25", sensorData.pm25);
    json.set("PM50", sensorData.pm50);
    json.set("PM100", sensorData.pm100);
    json.set("timestamp/.sv", "timestamp");
    
    String dataPath = databasePath + getFormattedDateTime(true) + getFormattedDateTime(false);
    
    if (Firebase.RTDB.setJSON(&fbdo, dataPath.c_str(), &json)) {
        Serial.printf("Data sent successfully\nPath: %s\nType: %s\n",
                     fbdo.dataPath().c_str(), fbdo.dataType().c_str());
    } else {
        Serial.printf("Failed to send data\nError: %s\n", fbdo.errorReason().c_str());
    }
    
    updateLED(false);
    delay(LED_BLINK_DURATION);
}

String getFormattedDateTime(bool isDate) {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        Serial.println("Failed to obtain time");
        return "";
    }
    
    char buffer[12];
    if (isDate) {
        snprintf(buffer, sizeof(buffer), "/%04d-%02d-%02d",
                timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    } else {
        snprintf(buffer, sizeof(buffer), "/%02d:%02d:%02d",
                timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    }
    return String(buffer);
}

void updateLED(bool state) {
    digitalWrite(LED_PIN, state);
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    Serial2.begin(115200, SERIAL_8N1, RXPIN, TXPIN);
    
    Serial.println("\n**** 7100 ESP32 Particle Sensor ****");
    Serial.println("**** Piera Systems              ****\n");
    
    setupWiFi();
    setupTime();
    setupFirebase();
    setupSensor();
}

void loop() {
    if (Serial2.available() && readSensorData()) {
        if (Firebase.ready() && WiFi.status() == WL_CONNECTED) {
            sendToFirebase();
        }
    }
}
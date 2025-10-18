/*******************************************************************
 * Biometric Attendance System with Web Server for Viewing Logs
 *
 * Full working version including enrollment and deletion.
 *
 * It uses the Adafruit Fingerprint Sensor library, WiFi, and WebServer.
 *
 * Hardware:
 * - ESP32 Development Board
 * - Fingerprint Sensor (e.g., R307, FPM10A)
 *
 * Connections (using Hardware Serial 2 on ESP32):
 * - Sensor TX -> ESP32 RX2 (GPIO 16)
 * - Sensor RX -> ESP32 TX2 (GPIO 17)
 * - Sensor VCC -> ESP32 3.3V or 5V
 * - Sensor GND -> ESP32 GND
 *
 *******************************************************************/

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include <WiFi.h>
#include <WebServer.h>
#include "time.h"
#include <vector>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 12 * 3600; // GMT+12 = 43200
const int daylightOffset_sec = 1 * 3600;  // Additional 1 hour for NZDT = 3600

HardwareSerial fingerSerial(2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);
WebServer server(80); // Web server runs on port 80

struct AttendanceRecord {
  String userID;
  String timestamp;
};

std::vector<AttendanceRecord> attendanceLog;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time configured via NTP for New Zealand.");

  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); } 
  }
  
  server.on("/", handleRoot); 
  server.begin();
  Serial.println("HTTP server started.");
  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ in your browser to see the log.");

  printMenu();
}

void loop() {
  server.handleClient(); 
  
  if (getFingerprintID() != -1) {
    takeAttendance();
  }
  
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case 'e':
        enrollFingerprint();
        break;
      case 'a':
        takeAttendance();
        break;
      case 'd':
        deleteFingerprint();
        break;
      default:
        Serial.println("Invalid command.");
        printMenu();
        break;
    }
  }
}


void handleRoot() {
  String html = "<!DOCTYPE html><html><head><title>Attendance Log</title>";
  html += "<style>body{font-family: Arial, sans-serif; margin: 20px;} h1{color: #333;} table{width: 60%; border-collapse: collapse; margin-top: 20px;} th, td{padding: 10px; text-align: left; border: 1px solid #ddd;} th{background-color: #f2f2f2;}</style>";
  html += "<meta http-equiv='refresh' content='10'>"; // Auto-refresh the page every 10 seconds
  html += "</head><body>";
  html += "<h1>Biometric Attendance Log</h1>";
  html += "<table><thead><tr><th>User ID</th><th>Timestamp</th></tr></thead><tbody>";
  
  for (const auto& record : attendanceLog) {
    html += "<tr><td>" + record.userID + "</td><td>" + record.timestamp + "</td></tr>";
  }
  
  html += "</tbody></table></body></html>";
  
  server.send(200, "text/html", html);
}


String getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "Failed to get time";
  }
  char timeString[50];
  strftime(timeString, sizeof(timeString), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return String(timeString);
}

void printMenu() {
  Serial.println("\n--- MENU ---");
  Serial.println("Place finger on sensor to mark attendance automatically.");
  Serial.println("Or enter a command in the Serial Monitor:");
  Serial.println("  'e' -> Enroll a new fingerprint.");
  Serial.println("  'd' -> Delete a fingerprint.");
  Serial.println("------------");
}


void takeAttendance() {
  int fingerID = getFingerprintID_blocking(); 
  
  if (fingerID != -1) {
    Serial.print("Attendance Marked! Welcome User #");
    Serial.println(fingerID);
    
    AttendanceRecord newRecord;
    newRecord.userID = "User #" + String(fingerID);
    newRecord.timestamp = getCurrentTime();
    attendanceLog.push_back(newRecord);
    
  } else {
    Serial.println("Finger not found. Please try again.");
  }
  printMenu();
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return -1;
  
  return finger.fingerID;
}

int getFingerprintID_blocking() {
  Serial.println("Please place your finger");
  uint8_t p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
  
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return -1;
  
  return finger.fingerID;
}

void enrollFingerprint() {
  Serial.println("Ready to enroll a new finger!");
  Serial.println("Please enter the ID # (1-127) to save this finger as...");
  
  int id = 0;
  while (id == 0) {
    while (!Serial.available());
    id = Serial.parseInt();
  }
  if (id < 1 || id > 127) {
    Serial.println("Invalid ID. Please choose between 1 and 127.");
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  Serial.println("Place your finger on the sensor...");
  int p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
  p = finger.image2Tz(1);
  if (p != FINGERPRINT_OK) { Serial.println("Image conversion failed"); return; }
  
  Serial.println("Remove your finger.");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  Serial.println("Place the same finger again...");
  p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
  }
  p = finger.image2Tz(2);
  if (p != FINGERPRINT_OK) { Serial.println("Image conversion failed"); return; }

  Serial.println("Creating model...");
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else {
    Serial.println("Fingerprints did not match.");
    return;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.print("Successfully stored model for ID #");
    Serial.println(id);
  } else {
    Serial.println("Failed to store model.");
  }
  printMenu();
}

void deleteFingerprint() {
  Serial.println("Please enter the ID # (1-127) you want to delete:");
  uint8_t id = 0;
  while (id == 0) {
    while (!Serial.available());
    id = Serial.parseInt();
  }

  uint8_t p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.print("Deleted ID #");
    Serial.println(id);
  } else {
    Serial.println("Failed to delete fingerprint.");
  }
  printMenu();
}
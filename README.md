# ESP32 Biometric Attendance System with Web Server

This project uses an ESP32 microcontroller and a fingerprint sensor to create a secure and automated attendance tracking system. Attendance records, including a user ID and an accurate timestamp, are stored and can be viewed in real-time on a web page hosted directly by the ESP32.



## Features

* **Fingerprint Management**: Enroll new users, identify existing users, and delete users from the sensor's memory via a simple serial interface.
* **Real-time Attendance Logging**: When a valid finger is scanned, the system logs the User ID and a timestamp.
* **Accurate Timekeeping**: Connects to an NTP (Network Time Protocol) server on boot to ensure timestamps are always accurate.
* **Built-in Web Server**: Hosts a simple, clean webpage that displays the attendance log in a table.
* **Live Updates**: The webpage automatically refreshes every 10 seconds to show the latest entries without needing a manual reload.
* **Non-blocking Operation**: The system can log attendance automatically just by placing a finger on the sensor, without needing a command.

***

## Hardware Required

* ESP32 Development Board
* Fingerprint Sensor (e.g., R307, FPM10A)
* Jumper Wires
* Breadboard (optional)

***

## Wiring Diagram

The fingerprint sensor is connected to the ESP32 using a hardware serial port (`Serial2`) for reliable communication.

| Fingerprint Sensor | ESP32 Pin |
| :----------------- | :---------- |
| **VCC** (Red) | `3.3V` or `5V` |
| **GND** (Black) | `GND` |
| **TX** (Green/White) | `GPIO 16` (RX2) |
| **RX** (White/Green) | `GPIO 17` (TX2) |



***

## 🚀 Getting Started

Follow these steps to get the project up and running.

### 1. Software Setup

* Ensure you have the [Arduino IDE](https://www.arduino.cc/en/software) installed.
* Install the ESP32 board support package. You can find instructions [here](https://docs.espressif.com/projects/arduino-esp32/en/latest/installing.html).
* In the Arduino IDE, go to **Tools > Library Manager** and install the **Adafruit Fingerprint Sensor Library**.

### 2. Configure the Code

* Clone or download this repository.
* Open the `.ino` file in the Arduino IDE.
* Update the Wi-Fi credentials with your network's details:
    ```cpp
    // --- Wi-Fi Credentials (IMPORTANT: CHANGE THESE) ---
    const char* ssid = "YOUR_WIFI_SSID";
    const char* password = "YOUR_WIFI_PASSWORD";
    ```
* (Optional) If you are not in New Zealand, adjust the time zone offset in seconds:
    ```cpp
    // Example for GMT-5
    const long gmtOffset_sec = -5 * 3600;
    const int daylightOffset_sec = 0; // Or 3600 if daylight saving applies
    ```

### 3. Upload to ESP32

* Connect your ESP32 to your computer.
* In the Arduino IDE, select your ESP32 board (e.g., "ESP32 Dev Module" or "Arduino Nano ESP32") and the correct COM port from the **Tools** menu.
* Click the **Upload** button.

***

## 💡 How to Use the System

### 1. System Management (Serial Monitor)

* Open the Arduino **Serial Monitor** and set the baud rate to **115200**.
* A menu will appear with instructions. You can type the following commands and press Enter:
    * `e` - **Enroll a new user**: The system will prompt you for a numeric ID (1-127) and guide you through the two-step scanning process.
    * `d` - **Delete a user**: The system will ask for the ID of the fingerprint you wish to remove.

### 2. Marking Attendance

* Simply place a previously enrolled finger on the sensor.
* The Serial Monitor will confirm the attendance with a "Welcome" message, and the record will be saved.

### 3. Viewing the Attendance Log

* After the ESP32 connects to Wi-Fi, its IP address will be printed in the Serial Monitor.
    ```
    WiFi connected!
    IP address: 192.168.1.105
    ```
* Open a web browser on any device (phone, laptop, etc.) connected to the **same Wi-Fi network**.
* Enter the IP address into the browser's address bar. The attendance log webpage will be displayed.

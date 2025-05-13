# ESP32 IoT Credential & Data Display System

This project uses an **ESP32 microcontroller** to store WiFi credentials and display data from **Firebase** on an **OLED screen**. It also monitors temperature and humidity using a **DHT11 sensor**.

---

## Features
- WiFi credential storage using EEPROM
- Captive portal for WiFi setup
- Firebase data display on OLED
- Temperature and humidity monitoring using DHT11

---

## Components
- ESP32
- DHT11 Sensor
- SSD1306 OLED Display
- Firebase Realtime Database

---

## Setup

1. **Clone the repository:**

```bash
git clone https://github.com/Mahmoudmu1/ESP32_IoT_Credential_Data_Display.git
cd ESP32_IoT_Credential_Data_Display
```

**Install Libraries:**

```
Firebase_ESP_Client  
Adafruit_GFX  
Adafruit_SSD1306  
DHT Sensor Library  
```

**Upload the Code:**

- Open the `.ino` file in Arduino IDE.  
- Enter WiFi and Firebase details.  
- Select the ESP32 board and COM port.  
- Click **Upload**.  

---

## Firebase Structure

```
/displayText - Text for OLED  
/dht/temperature - Temperature data  
/dht/humidity - Humidity data  
```

---

## Example Output

**OLED Display:**

```
WiFi: OK    User: Mahmud  
T: 25.3C  H: 60%  
Message from Firebase  
```

**Serial Monitor:**

```
Connecting to WiFi...  
WiFi Connected  
Temp: 25.3 C, Humidity: 60%  
```

---

## Repository Structure

```
/ESP32_IoT_Credential_Data_Display/
├── ESP32_IoT_Credential_Data_Display.ino
├── README.md
└── libraries/
    ├── Firebase_ESP_Client/
    └── Adafruit_SSD1306/
```

---

**Author:** Mahmoud Musa Uthman

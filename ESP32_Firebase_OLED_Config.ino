#include <WiFi.h>
#include <EEPROM.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>

// Define DHT sensor pin and type
#define DHTPIN 4
#define DHTTYPE DHT11

// OLED Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

// EEPROM addresses for storing WiFi credentials and username
#define EEPROM_SIZE 128
#define SSID_ADDR 0
#define PASS_ADDR 32
#define USER_ADDR 64

// Firebase configuration
#define API_KEY "AIzaSyDGRm22frjc80mHz9xRFpgIing6Bwg0gOk"
#define DATABASE_URL "https://sensorbased-e58f9-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Create instances of the sensor and display objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
AsyncWebServer server(80);
DNSServer dns;

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;

// Function to write a string to EEPROM
void writeStringToEEPROM(int addr, const String &str) {
  for (int i = 0; i < 32; ++i)
  // Write each character to the specified address
    EEPROM.write(addr + i, i < str.length() ? str[i] : 0);
  EEPROM.commit(); // Save the changes to EEPROM
}

// Function to read a string from EEPROM
String readStringFromEEPROM(int addr) {
  char data[33]; // Buffer to hold data read from EEPROM
  for (int i = 0; i < 32; ++i) 
    data[i] = EEPROM.read(addr + i); // Read each byte
  data[32] = '\0'; // Null-terminate the string
  return String(data);
}

// Function to start the captive portal for WiFi and user configuration
void startCaptivePortal() {
  // Start access point with the SSID "ESP32_Config_Mahmud"
  WiFi.softAP("ESP32_Config_Mahmud", "");
  IPAddress IP = WiFi.softAPIP();
  dns.start(53, "*", IP); // Redirect all requests to the access point

  // Serve configuration page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String alert = "";
    if (request->hasParam("status")) {
      String status = request->getParam("status")->value();
      if (status == "updated") {
        alert = "<p style='color:green;text-align:center;'>✅ Text updated!</p>";
      }
    }

    String html = R"rawliteral(
      <!DOCTYPE html>
      <html>
      <head>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <title>ESP32 Config Portal</title>
        <style>
          body {
            font-family: Arial, sans-serif;
            background: #eef2f7;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
          }
          .container {
            background-color: white;
            padding: 20px;
            border-radius: 10px;
            width: 90%;
            max-width: 320px;
            box-shadow: 0px 4px 12px rgba(0, 0, 0, 0.1);
          }
          h2 {
            text-align: center;
            margin-bottom: 10px;
          }
          form {
            margin-bottom: 20px;
          }
          input[type="text"], input[type="password"] {
            width: 100%;
            padding: 10px;
            margin: 8px 0;
            border: 1px solid #ccc;
            border-radius: 5px;
            box-sizing: border-box;
          }
          input[type="submit"] {
            width: 100%;
            background-color: #007BFF;
            color: white;
            padding: 10px;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            margin-top: 5px;
          }
          input[type="submit"]:hover {
            background-color: #0056b3;
          }
        </style>
      </head>
      <body>
        <div class="container">
          <h2>WiFi & User Setup</h2>
          <form action="/save">
            <input type="text" name="ssid" placeholder="WiFi SSID" required>
            <input type="password" name="pass" placeholder="WiFi Password">
            <input type="text" name="user" placeholder="Username" required>
            <input type="submit" value="Save WiFi & Reboot">
          </form>

          <h2>Update OLED Text</h2>
          <form action="/update">
            <input type="text" name="text" placeholder="New display text" required>
            <input type="submit" value="Update Text">
          </form>
    )rawliteral";

    html += alert;
    html += R"rawliteral(
        </div>
      </body>
      </html>
    )rawliteral";

    request->send(200, "text/html", html);
  });

// Handle saving of WiFi credentials and user data
  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request) {
    String ssid = request->getParam("ssid")->value();
    String pass = request->getParam("pass")->value();
    String user = request->getParam("user")->value();

    // Store credentials in EEPROM
    writeStringToEEPROM(SSID_ADDR, ssid);
    writeStringToEEPROM(PASS_ADDR, pass);
    writeStringToEEPROM(USER_ADDR, user);

    // Notify user and restart device
    request->send(200, "text/html", "✅ WiFi credentials saved! Rebooting...");
    delay(1000);
    ESP.restart(); // Restart to apply new settings
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("text")) {
      String newText = request->getParam("text")->value();
      Firebase.RTDB.setString(&fbdo, "/displayText", newText);
      request->redirect("/?status=updated");
    }
  });

  server.begin(); // Start the web server
}

// Setup function - runs once at startup
void setup() {
  Serial.begin(115200);
  dht.begin();
  EEPROM.begin(EEPROM_SIZE);

// Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (true);
  }

// Display startup message on OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  // Start the captive portal
  startCaptivePortal();

  // Read WiFi credentials and user data from EEPROM
  String ssid = readStringFromEEPROM(SSID_ADDR);
  String pass = readStringFromEEPROM(PASS_ADDR);
  String user = readStringFromEEPROM(USER_ADDR);

   // Attempt to connect to WiFi
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.print("Connecting to WiFi");
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry++ < 20) {
    delay(500);
    Serial.print(".");
  }

  // Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected");
    Serial.printf("User: %s\n", user.c_str());

    // Configure Firebase settings
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    config.token_status_callback = tokenStatusCallback;

    // Attempt Firebase sign-up
    if (Firebase.signUp(&config, &auth, "", "")) {
      Serial.println("✅ Firebase signUp successful");
      signupOK = true;
    } else {
      Serial.printf("❌ SignUp failed: %s\n", config.signer.signupError.message.c_str());
    }

    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
  } else {
    Serial.println("\n❌ WiFi connection failed.");
  }

  // Display system ready message on OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("System Ready.");
  display.display();
  delay(1000);
}

// Main loop - continuously runs after setup
void loop() {
  // Read temperature and humidity data from DHT sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  String displayText = "N/A";
  String user = readStringFromEEPROM(USER_ADDR);

  // Check if sensor data is valid
  if (!isnan(temp) && !isnan(hum)) {
    Firebase.RTDB.setFloat(&fbdo, "/dht/temperature", temp);
    Firebase.RTDB.setFloat(&fbdo, "/dht/humidity", hum);
  }

  // Fetch display text from Firebase
  if (Firebase.RTDB.getString(&fbdo, "/displayText")) {
    displayText = fbdo.stringData();
  }

  // Update OLED display with WiFi status, user, temperature, humidity, and message
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("WiFi: %s", WiFi.status() == WL_CONNECTED ? "OK" : "Fail");

  display.setCursor(64, 0); 
  display.print("U:");
  display.print(user.substring(0, 10)); // Display first 10 characters of username

  display.setCursor(0, 10);
  display.printf("T:%.1fC H:%.1f%%", temp, hum); // Display temperature and humidity

  display.setCursor(0, 20);
  display.print(displayText.substring(0, 20)); // Display message from Firebase
  display.display();

  delay(5000); // Delay before next data read
}

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <ThingSpeak.h>
#include <SPI.h>
#include <MFRC522.h>

// Custom I2C pins for ESP32
#define I2C_SDA 5
#define I2C_SCL 22

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi credentials
const char* ssid = "Sam32";  // Replace with your network SSID
const char* password = "12345678";  // Replace with your network password

// ThingSpeak settings
WiFiClient client;

// RFID settings
#define SS_PIN 21 // SDA pin
#define RST_PIN 15 // RST pin
MFRC522 rfid(SS_PIN, RST_PIN); // Create MFRC522 instance

// Farmer details
struct Farmer {
  String name;
  String customerNumber;  // This will hold the UID for validation
  String centerNumber;
  String apiKey;
};

// Define farmers
Farmer farmers[2] = {
  {"OMKAR", "af2f180", "Center01", "Y2FN1P58U8K80Z6R"}, // Farmer 1
  {"RAHUL", "9246da51", "Center02", "Y2FN1P58U8K80Z6R"}  // Farmer 2
};

Farmer currentFarmer;  // To store the current farmer's details
String incomingData = "";  // String to hold incoming data
bool collectingData = false;  // Flag to indicate when to collect data
bool processDataFlag = false;  // Flag to indicate data processing should happen

// Variables to hold the milk analyzer data
float FAT = 0;
float SNF = 0;
float CLR = 0;
float WaterContent = 0;
float LACTOSE = 0;
float Protein = 0;

void setup() {
  // Initialize Serial communication for debugging
  Serial.begin(2400);
  
  // Initialize Serial2 for external device communication (Set RX to GPIO 16, TX to GPIO 17, and baud rate to 2400)
  Serial2.begin(2400, SERIAL_8N1, 16, 17);

  // Initialize I2C for OLED using custom pins
  Wire.begin(I2C_SDA, I2C_SCL);
  
  // Initialize OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Use the correct I2C address (0x3C)
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Infinite loop
  }

  display.display();
  delay(2000);

  // Display welcome message
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("WELCOME TO PROJECT");
  display.println("IoT Based Milk Analyzer");
  display.display();
  delay(2000);

  // Connect to WiFi (if applicable)
  connectToWiFi();

  // Initialize ThingSpeak (if applicable)
  ThingSpeak.begin(client);

  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();
  Serial.println("Place your card near the reader...");
}

void loop() {
  // Look for a card
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    Serial.println("RFID successfully detected!");

    // Read UID
    String uid = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      uid += String(rfid.uid.uidByte[i], HEX);
    }
    Serial.print("UID: ");
    Serial.println(uid);

    // Check against known farmers
    bool foundFarmer = false;
    for (int i = 0; i < 2; i++) {
      if (uid == farmers[i].customerNumber) {
        currentFarmer = farmers[i]; // Store the detected farmer's details
        displayFarmerDetails(currentFarmer);
        processDataFlag = true;  // Set flag to indicate data collection and ThingSpeak transmission
        foundFarmer = true;
        Serial.print("Farmer identified: ");
        Serial.println(currentFarmer.name);
        break;
      }
    }

    if (!foundFarmer) {
      displayGuestDetails(uid); // If not found, display as guest
      Serial.println("Guest detected.");
    }

    // Halt PICC
    rfid.PICC_HaltA();
  }

  // If RFID has been scanned, process incoming data from Serial2 (external device)
  if (processDataFlag) {
    Serial.println("Processing incoming data from Serial2...");
    while (Serial2.available()) {
      char receivedChar = Serial2.read();
      if (receivedChar == '(') {
        incomingData = "";
        collectingData = true;
        Serial.println("Start collecting data...");
      }

      if (collectingData && receivedChar == ')') {
        collectingData = false;
        Serial.println("End of data received: " + incomingData);
        if (processReceivedString(incomingData)) { // Process and check for success
          displayMilkParameters();  // Display parameters after processing data
          Serial.println("Parameters displayed. Sending to ThingSpeak...");
          sendToThingSpeak();  // Send data to ThingSpeak after processing
        } else {
          Serial.println("Data processing failed.");
        }
        processDataFlag = false;  // Reset the flag after data is sent
      }

      if (collectingData && receivedChar != '(' && receivedChar != ')') {
        incomingData += receivedChar;
      }

      delay(10);
    }
  }
}

// Display farmer details on OLED
void displayFarmerDetails(Farmer farmer) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Farmer: " + farmer.name);
  display.println("Cust No: " + farmer.customerNumber);
  display.println("Center No: " + farmer.centerNumber);
  display.display();
}

// Display guest details on OLED
void displayGuestDetails(String uid) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Guest detected!");
  display.println("UID: " + uid);
  display.display();
}

// Display milk parameters on OLED
void displayMilkParameters() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("FAT: ");
  display.print(FAT, 2);
  display.print("\nSNF: ");
  display.print(SNF, 2);
  display.print("\nCLR: ");
  display.print(CLR, 2);
  display.print("\nWater: ");
  display.print(WaterContent, 2);
  display.print("\nLactose: ");
  display.print(LACTOSE, 2);
  display.print("\nProtein: ");
  display.print(Protein, 2);
  display.display();
}

// Connect to WiFi
void connectToWiFi() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Connecting to WiFi...");
  display.display();
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
  int wifiRetryCount = 0;

  while (WiFi.status() != WL_CONNECTED && wifiRetryCount < 20) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    wifiRetryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Connected!");
    display.display();
    delay(2000);
  } else {
    Serial.println("\nFailed to Connect to WiFi");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("WiFi Connection Failed");
    display.display();
    while (true);
  }
}

// Process received string and extract data
bool processReceivedString(String data) {
  data.trim();
  data.replace("(", "");
  data.replace(")", "");

  int index = 0;
  String decimalValues[6];
  int valueIndex = 0;

  while (index < data.length() && valueIndex < 6) {
    String chunk = data.substring(index, index + 4);
    int decimalValue = chunk.toInt();
    decimalValues[valueIndex++] = String(decimalValue);
    index += 4;
  }

  if (valueIndex == 6) {
    FAT = decimalValues[0].toFloat() / 100;
    SNF = decimalValues[1].toFloat() / 100;
    CLR = decimalValues[2].toFloat() / 100;
    WaterContent = decimalValues[3].toFloat() / 100;
    LACTOSE = decimalValues[4].toFloat() / 100;
    Protein = decimalValues[5].toFloat() / 100;
    Serial.println("Data processed successfully:");
    Serial.print("FAT: "); Serial.println(FAT);
    Serial.print("SNF: "); Serial.println(SNF);
    Serial.print("CLR: "); Serial.println(CLR);
    Serial.print("WaterContent: "); Serial.println(WaterContent);
    Serial.print("LACTOSE: "); Serial.println(LACTOSE);
    Serial.print("Protein: "); Serial.println(Protein);
    return true;
  }

  Serial.println("Error processing data");
  return false;
}

// Send data to ThingSpeak
void sendToThingSpeak() {
  ThingSpeak.setField(1, FAT);
  ThingSpeak.setField(2, SNF);
  ThingSpeak.setField(3, CLR);
  ThingSpeak.setField(4, WaterContent);
  ThingSpeak.setField(5, LACTOSE);
  ThingSpeak.setField(6, Protein);

  int responseCode = ThingSpeak.writeFields(0, currentFarmer.apiKey.c_str()); // Send data to the correct channel

  if (responseCode == 200) {
    Serial.println("Data sent successfully to ThingSpeak!");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Data Sent!");
    display.display();
  } else {
    Serial.print("Failed to send data. HTTP Error code: ");
    Serial.println(responseCode);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Failed to send data:");
    display.println(responseCode);
    display.display();
  }
}

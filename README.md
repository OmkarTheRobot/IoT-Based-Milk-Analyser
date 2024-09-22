# IoT-Based-Milk-Analyser
This project is an IoT-based milk analyzer using ESP32, RFID, and an OLED display to identify farmers and measure milk quality parameters like FAT, SNF, and CLR. Data is displayed on the OLED and sent to ThingSpeak for real-time monitoring.
****
IoT-Based Milk Analyzer with RFID Integration and ThingSpeak Communication
Project Overview

This project is an IoT-based milk analyzer system designed to read milk quality parameters using a sensor module and send the data to the ThingSpeak IoT platform for remote monitoring. The system also integrates an RFID reader (MFRC522) to identify farmers based on their RFID cards, making it easy to associate milk data with individual farmers.
Key Features

    RFID Authentication: The system uses an RFID reader (MFRC522) to identify farmers based on their RFID card's unique ID (UID). If the card matches a pre-registered farmer, the system associates milk data with that farmer. Otherwise, it registers the user as a guest.

    Milk Quality Parameters: The system captures key milk quality parameters including:
        FAT
        SNF (Solid Not Fat)
        CLR (Clear Liquid Ratio)
        Water Content
        Lactose
        Protein

    OLED Display: A 128x64 OLED display shows the farmer's details and milk quality parameters in real-time.

    ThingSpeak Integration: The captured milk data is sent to the ThingSpeak IoT platform for real-time monitoring and data logging, using the individual farmer's API key for secure data transmission.

Components Used

    ESP32: The main microcontroller for handling the entire system, including RFID reading, serial communication, and WiFi-based data transmission.
    RFID Reader (MFRC522): Used to identify farmers based on RFID card UIDs.
    OLED Display (128x64 SSD1306): Displays farmer information and milk quality parameters.
    Milk Analyzer: Communicates via Serial2 (UART) to send milk parameter data.
    ThingSpeak: Used to log milk parameter data to the cloud via WiFi.


Hardware Setup
Component	Pin Type	ESP32 Pin Number	Description
OLED Display	SDA	5	I2C Data Line
SCL	22	I2C Clock Line
RFID Reader	SDA (SS)	21	Slave Select for SPI (Chip Select)
RST	15	Reset pin for the MFRC522 module
MOSI	23	Master Out Slave In (SPI data out)
MISO	19	Master In Slave Out (SPI data in)
SCK	18	Serial Clock for SPI
Serial2 (Milk Analyzer)	RX	16	Receive pin for external device (Milk Analyzer)
TX	17	Transmit pin for external device

Software Setup

    RFID Identification: Each farmer's RFID card is identified based on a pre-registered UID. If the RFID card matches the stored UID, the system displays the farmer's name and details.

    Milk Parameter Processing: The system reads incoming serial data from the milk analyzer, processes the data (e.g., FAT, SNF, CLR), and displays the results on the OLED display.

    ThingSpeak Data Upload: The milk quality parameters are sent to the ThingSpeak IoT platform for logging and analysis.

How to Use

    Clone the Repository:

    bash

    git clone https://github.com/yourusername/iot-milk-analyzer

    Upload Code: Flash the code onto the ESP32 using the Arduino IDE or PlatformIO.

    Hardware Setup: Wire the RFID module, OLED display, and milk analyzer sensor as per the pin connections in the table above.

    Connect to WiFi: Ensure the ESP32 is connected to a WiFi network by editing the SSID and password in the code.

    Run the System: The system will detect the RFID card, gather milk parameters, and upload them to ThingSpeak.

Future Improvements

    Expand the system to support multiple farmers.
    Add support for more milk parameters.
    Integrate mobile notifications when new data is uploaded to ThingSpeak.

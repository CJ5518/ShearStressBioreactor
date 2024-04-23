#include "routineManager.hpp"
#include "GUI.hpp"
#include "thingSpeak.hpp"

// Used for sending and receiving Modbus commands
HardwareSerial ModbusSerial(1);

// Receive and transmit pins for the MAX485
const int MODBUS_RX = 16;
const int MODBUS_TX = 17;
const int MODBUS_ENABLE = 18; // automatically set to high when writing, low otherwise to receive

YAAJ_ModbusMaster controller;

RoutineManager rm;
Scheduler ts;
GUI gui;

const unsigned long postingInterval = 15000; // Interval at which to post (in milliseconds)
unsigned long lastConnectionTime = 0; // Last time data was posted to ThingSpeak

void setup() {
    Serial.begin(115200); // for USB debugging
    // Serial connection for sending RS485 commands
    controller.begin(ModbusSerial, 9600, SERIAL_8N1, MODBUS_RX, MODBUS_TX, 0xEF, MODBUS_ENABLE, 500);
    while (!Serial || !ModbusSerial) {} // wait until connections are ready

    // RoutineManager initialization
    rm.init(&ts, false);
    //rm.testControl(); // pump control testing
    //rm.collectFlowRates(); // testing for flow sensor data

    // Init GUI, passing in RoutineManager instance pointer
    gui.init(&ts, &rm);
    Serial.println("Setup complete.");

    /*
        ThingSpeak section
        For University of Idaho, to bypass daily re-authentication on VandalGuest network, remember to register
        the ESP32's MAC address with UI system at help.uidaho.edu/avsetup
        (optional) Print ESP32 IP address and MAC address:
            Serial.print("IP Address: "); Serial.println(WiFi.localIP());
            Serial.print("MAC Address: "); Serial.println(WiFi.macAddress());
    
        // Assuming Wi-Fi credentials are already assigned in ssid and password (either directly in code or through GUI)
        Serial.print("Connecting to "); Serial.println(ssid);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(".");
        }
        Serial.println("");
        Serial.println("WiFi connected");
    */
    
    pinMode(LED_BUILTIN, OUTPUT); // Built-in blue LED on the ESP32 (using this to confirm system ON/OFF)
}

void loop() {
    ts.execute();
    
    // send to ThingSpeak section (field1, field2, and field7)
    if (WiFi.status() == WL_CONNECTED)
    {
        // Measure WiFi signal strength and print
        int32_t rssi = WiFi.RSSI();
        Serial.print("Signal strength (RSSI): ");
        Serial.println(rssi);

        // LED turn on when Wi-Fi connected, update LED status on ThingSpeak as well
        digitalWrite(LED_BUILTIN, HIGH);
        int ledStatus = 1;
        sendToThingSpeak_field2(ledStatus);

        // Check if it's time to send a new measurement to ThingSpeak
        unsigned long currentTime = millis();
        if (currentTime - lastConnectionTime >= postingInterval)
        {
            // Send the data to ThingSpeak
            sendToThingSpeak_field1(rssi);
        }
    } 
    else 
    {
        // LED turn off when Wi-Fi is not connected, update LED status on ThingSpeak as well
        digitalWrite(LED_BUILTIN, LOW);
        int ledStatus = 0;
        sendToThingSpeak_field2(ledStatus);

        // Not connected to WiFi, attempt to reconnect
        Serial.println("WiFi not connected, attempting to reconnect...");
        WiFi.begin(ssid, password);
    }

    unsigned long runTime = millis();
    sendToThingSpeak_field7(runTime);
    
    delay(15000); // Send to ThingSpeak every 15 seconds
}
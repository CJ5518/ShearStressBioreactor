/*  
    This is ThingSpeak secction, remove or add comments accordingly to run properly. Make sure ESP32 is connect to Wi-Fi router in order to push data to ThingSpeak.
    Create your own account and channel at ThingSpeak.com
    For this specific Bioreactor Project channel, login with
     ID: bui2848@vandals.uidaho.edu
     PW: Bioreactor@24
    Channel infomation if needed:
        Link: https://thingspeak.com/channels/2479428/
        Channel number: 2479428
        Write API Key: QXK1OGL1BSWQ4LPP
        Read API Key: LRSWKH2NV2MAPBW0
        Write to channel field: https://api.thingspeak.com/update?api_key=QXK1OGL1BSWQ4LPP&field1=0 //replace field number and value accordingly, refer to function sendToThingSpeak below
        All field list:
            1/ Wi-Fi Signal
            2/ ESP32 LED
            3/ Actual Flow Rate
            4/ Target Flow Rate
            5/ Pump Flow Rate
            6/ Estimated Shear Stress
            7/ System Runtime
            8/ Calculated Reynold's number
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <ThingSpeak.h>
#include "thingSpeak.hpp"

//const char *ssid = "AirVandalGuest";       // Replace with your WiFi SSID
//const char *password = "GoVandals!";       // Replace with your WiFi password
const char *host = "api.thingspeak.com";    // ThingSpeak API endpoint

WiFiClient client;

ThingSpeak::init()

// Send Wi-Fi signal strength data to ThingSpeak (update field 1)
void sendToThingSpeak_field1(int32_t rssi)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url1 = "/update?api_key=QXK1OGL1BSWQ4LPP&field1=" + String(rssi); // Write to field 1 Wi-Fi signal strength, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 1: ");
        Serial.println(url1);
        client.print(String("GET ") + url1 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    } 
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}

// Send LED status to ThingSpeak (update field 2)
void sendToThingSpeak_field2(int ledStatus)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url2 = "/update?api_key=QXK1OGL1BSWQ4LPP&field2=" + String(ledStatus); // Write to field 2 LED status, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 2: ");
        Serial.println(url2);
        client.print(String("GET ") + url2 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}


// Send Actual Flow Rate status to ThingSpeak (update field 3)
void sendToThingSpeak_field3(double actualFlow)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url3 = "/update?api_key=QXK1OGL1BSWQ4LPP&field3=" + String(actualFlow); // Write to field 3 actual flow rate, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 3: ");
        Serial.println(url3);
        client.print(String("GET ") + url3 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}

// Send Target Flow Rate status to ThingSpeak (update field 4)
void sendToThingSpeak_field4(double targetFlow)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url4 = "/update?api_key=QXK1OGL1BSWQ4LPP&field4=" + String(targetFlow); // Write to field 4 target flow rate, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 4: ");
        Serial.println(url4);
        client.print(String("GET ") + url4 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}

// Send Pump Flow Rate status to ThingSpeak (update field 5)
void sendToThingSpeak_field5(double pumpFlow)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url5 = "/update?api_key=QXK1OGL1BSWQ4LPP&field5=" + String(pumpFlow); // Write to field 5 pump flow rate, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 5: ");
        Serial.println(url5);
        client.print(String("GET ") + url5 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}

// Send Estimated Shear status to ThingSpeak (update field 6)
void sendToThingSpeak_field6(double shearStress)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url6 = "/update?api_key=QXK1OGL1BSWQ4LPP&field6=" + String(shearStress); // Write to field 6 estimated shear stress, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 6: ");
        Serial.println(url6);
        client.print(String("GET ") + url6 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}

// Send Runtime status to ThingSpeak (update field 7)
void sendToThingSpeak_field7(long runTime)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url7 = "/update?api_key=QXK1OGL1BSWQ4LPP&field7=" + String(runTime); // Write to field 7 system runtime, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 7: ");
        Serial.println(url7);
        client.print(String("GET ") + url7 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}

// Send Calculated Reynold's number status to ThingSpeak (update field 8)
void sendToThingSpeak_field8(double calculatedR)
{
    Serial.println("Sending data to ThingSpeak...");

    if (client.connect(host, 80))
    {
        String url8 = "/update?api_key=QXK1OGL1BSWQ4LPP&field8=" + String(calculatedR); // Write to field 8 calculated Reynold's number, remember to replace with your ThingSpeak API key
        Serial.print("Requesting URL 8: ");
        Serial.println(url8);
        client.print(String("GET ") + url8 + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
        lastConnectionTime = millis();
    }
    else
    { Serial.println("Connection to ThingSpeak failed"); }
}
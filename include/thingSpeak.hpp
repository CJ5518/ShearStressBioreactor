#pragma once

#include "routineManager.hpp"
#include "utils.hpp"

class ThingSpeak
{
    public:
        // Declare the constructor
        ThingSpeak();
        void init();

        // Send Wi-Fi signal strength data to ThingSpeak (update field 1)
        void sendToThingSpeak_field1(int32_t rssi);

        // Send LED status to ThingSpeak (update field 2)
        void sendToThingSpeak_field2(int ledStatus);

        // Send Actual Flow Rate status to ThingSpeak (update field 3)
        void sendToThingSpeak_field3(double actualFlow);

        // Send Target Flow Rate status to ThingSpeak (update field 4)
        void sendToThingSpeak_field4(double targetFlow);

        // Send Pump Flow Rate status to ThingSpeak (update field 5)
        void sendToThingSpeak_field5(double pumpFlow);

        // Send Estimated Shear status to ThingSpeak (update field 6)
        void sendToThingSpeak_field6(double shearStress);

        // Send Runtime status to ThingSpeak (update field 7)
        void sendToThingSpeak_field7(long runTime);

        // Send Calculated Reynold's number status to ThingSpeak (update field 8)
        void sendToThingSpeak_field8(double calculatedR);

        // Collect Wi-Fi signal strength and change LED on ESP32 depends on connection
        void getWifiAndLed();

        // Collect the Runtime of the system
        void getSystemRuntime();
};
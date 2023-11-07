#include <Arduino.h>
#include "../include/project_includes.hpp"
#include <Wire.h>
const int dual_sensor_sys = 0;
const bool isWater = true;

bool low_fs_active = false;
bool debug = false; // debug mode

Pump p_1;
FlowManager fm;
RoutineManager rm;

int loopCount = 0;

// Variables for testing flow rates
float lowFlowRateList[] = {2.00, 5.00, 10.00, 15.00, 20.00};
const int lowFlowLen = *(&lowFlowRateList + 1) - lowFlowRateList; // length of test array
int lowFlowTimeResults[lowFlowLen];
float highFlowRateList[] = {200.0, 250.0, 300.0, 350.0, 400.0};
const int highFlowLen = *(&highFlowRateList + 1) - highFlowRateList;
int highFlowTimeResults[highFlowLen];

int lowFlowRateTimeList[lowFlowLen];
int highFlowRateTimeList[highFlowLen];

int lowTickList[lowFlowLen];
int highTickList[highFlowLen];
int tickCount;

int start_time;
int end_time;

float flow;
int temp;

/*
 * Start serial connection and initialize the pump and flow manager.
 */
void setup() {
    Serial.begin(115200);
    Wire.begin();
    while(!Serial){} // wait until connection is ready

    p_1.init(); // initialize the pump
    fm.init(); // initialize flow manager
    rm.init(&fm, &p_1, false); // TODO eventually only the routine manager needs to control the flow manager and maybe pump too

    delay(50);
}

char rx_byte = 0;
char Buffer[50];
char Command[5];
char Data[20];
int ByteCount;
bool checkInput;
int c;

bool greenLightFlag = false;
bool readyState = false;

/*
 * If the debug flag is set, test the preprogrammed flow rate lists. Otherwise, wait for commands from serial input.
 */
void loop() {
    if(debug) {
        delay(2000); // wait for pump to push enough flow through the system
        if(loopCount == 0) {            
            Serial.println("Testing low flow rate list");

            // Run flow tests with list of low flow rates
            for(int i = 0; i < lowFlowLen; i++) {
                start_time = millis();
                fm.test_flow(lowFlowRateList[i], true);
                end_time = millis(); // measure the time the test_flow() takes

                lowFlowRateTimeList[i] = (end_time - start_time) / 1000;

                lowTickList[i] = fm.stepsTaken;
                delay(2000);

                Serial.println("Closing flow: ");
                if(lowFlowRateList[i] > 30.0) {
                    fm.close_flow(false);
                }
                else {
                    fm.close_flow(true);
                }
            }

            Serial.println("Finished flow test!\nPrinting results...");
            for(int i = 0; i < lowFlowLen; i++) {
                Serial.print("Flow rate tgt: ");
                Serial.print(lowFlowRateList[i]);
                Serial.print(" time: ");
                Serial.print(lowFlowRateTimeList[i]);
                Serial.print(" seconds ");
                Serial.print(" ticks: ");
                Serial.print(lowTickList[i]);
                Serial.println();
            }

            loopCount++;
        }

        if(loopCount == 1) {
            Serial.println("Testing high flow rate list");

            // Run flow tests with list of high flow rates
            for(int i = 0; i < highFlowLen; i++) {
                start_time = millis();
                fm.test_flow(highFlowRateList[i], true);
                end_time = millis();

                highFlowRateTimeList[i] = (end_time - start_time) / 1000;

                highTickList[i] = fm.stepsTaken;
                delay(2000);

                // Close the valve, though this is not great since the pump is still on
                Serial.println("Closing flow: ");
                if(highFlowRateList[i] > 30.0) {
                    fm.close_flow(false);
                }
                else {
                    fm.close_flow(true);
                }
            }

            Serial.println("Finished flow test!\nPrinting results...");
            for(int i = 0; i < highFlowLen; i++) {
                Serial.print("Flow rate tgt: ");
                Serial.print(highFlowRateList[i]);
                Serial.print(" time: ");
                Serial.print(highFlowRateTimeList[i]);
                Serial.print(" seconds ");
                Serial.print(" ticks: ");
                Serial.print(highTickList[i]);
                Serial.println();
                
            }
            loopCount++;
        }
    }
    // Normal operation
    else {
        // Connect to master device
        while(!greenLightFlag) {
            if(Serial.available() > 0) {
                rx_byte = Serial.read();

                if(rx_byte == '1') {
                    greenLightFlag = true;
                }
            }
        }

        // Wait for system to be primed
        while(!readyState) {
            if(Serial.available() > 0) {
                rx_byte = Serial.read();

                switch(rx_byte) {
                    case '7':
                        // System is primed
                        readyState = true;
                        break;
                    case '1':
                        // Turn on the pump
                        p_1.togglePump(true);
                        break;
                    case '0':
                        // Turn off the pump
                        p_1.togglePump(false);
                        break;
                    default:
                        break;
                }
            }
        }
        Serial.write("1");

        // Connected and pump is primed
        while(greenLightFlag && readyState) {
            ByteCount = -1;
            ByteCount = Serial.readBytesUntil('\n', Buffer, sizeof(Buffer) - 1);

            // If anything was read
            if (ByteCount > 0) {
                // Parse the buffer
                if(strstr(Buffer, ",")) { // if there is a comma in the buffer
                    strcpy(Command, strtok(Buffer, ",")); // copy first part as command
                    strcpy(Data, strtok(NULL, "\n")); // copy second part as data
                }
                else { // only command sent through buffer
                    strcpy(Command, strtok(Buffer, "\n"));
                }

                // Get the integer the command represents
                if(!isspace(Command[0])) {
                    c = atoi(Command);
                    checkInput = true;
                }
                else {
                    checkInput = false;
                }
            }

            // If an integer command was provided
            if(checkInput) {
                switch (c) {
                    case 9:
                        // Close the valve in use
                        if(fm.flowAvg > 30.0) {
                            fm.close_flow(false);
                        }
                        else {
                            fm.close_flow(true);
                        }
                        
                        // Turn off the pump
                        p_1.togglePump(false);

                        Serial.println("9");
                        break;
                    case 2:
                        temp = atoi(Data);

                        // Turn on the pump if 1 is provided as the data
                        if(temp == 1) {
                            p_1.togglePump(true);
                        }
                        else {
                            p_1.togglePump(false);
                        }

                        Serial.println("2");
                        break;
                    case 1:
                        // Test the provided flow rate
                        flow = atoi(Data);
                        fm.test_flow(flow, true);
                        Serial.write("1");
                        break;
                    default:
                        break;
                }
                checkInput = false;
            }

            memset(Buffer, '\0', sizeof(Buffer));
            memset(Command, '\0', sizeof(Command));
            memset(Data, '\0', sizeof(Data));
        }
    }
}

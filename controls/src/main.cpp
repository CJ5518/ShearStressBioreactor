#include <Arduino.h>
#include "../include/project_includes.hpp"
#include <Wire.h>
const int dual_sensor_sys = 0;
const bool isWater = true;

bool low_fs_active = false;
bool debug = true; // debug mode


Pump p_1;

flowManager fm;

int loopCount = 1;

float lowFlowRateList[] = {2.00, 5.00, 10.00, 15.00, 20.00};
//float lowFlowRateList[] = {15.00, 20.00};
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
void setup() 
{
    Serial.begin(115200);
    Wire.begin();
    while(!Serial){}

    p_1.init();
    p_1.togglePump(false);

    fm.init(); // initialize flow manager

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

void loop() 
{
    if(debug)
    {
        //int i;
        delay(2000); // wait for pump to push enough flow through the system

        // * prime the pump

        if(loopCount == 0)
        {
            /*
            fm.test_flow(5.00, true);
            Serial.print("Total ticks: ");
            Serial.print(fm.stepsTaken);
            Serial.println();
            */
            
            Serial.println("Testing low flow rate list");

            // run flow tests with list of flow rates
            for(int i = 0; i < lowFlowLen; i++)
            {
                start_time = millis();
                fm.test_flow(lowFlowRateList[i], true);
                end_time = millis();

                lowFlowRateTimeList[i] = (end_time - start_time) / 1000;

                lowTickList[i] = fm.stepsTaken;
                delay(2000);

                Serial.println("Closing flow: ");
                if(lowFlowRateList[i] > 30.0)
                {
                    fm.close_flow(false);
                }
                else
                {
                    fm.close_flow(true);
                }
            }

            Serial.println("Finished flow test!\nPrinting results...");
            for(int i = 0; i < lowFlowLen; i++)
            {
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

        if(loopCount == 1)
        {
            Serial.println("Testing high flow rate list");

            // run flow tests with list of flow rates
            for(int i = 0; i < highFlowLen; i++)
            {
                start_time = millis();
                fm.test_flow(highFlowRateList[i], true);
                end_time = millis();

                highFlowRateTimeList[i] = (end_time - start_time) / 1000;

                highTickList[i] = fm.stepsTaken;
                delay(2000);

                Serial.println("Closing flow: ");
                if(highFlowRateList[i] > 30.0)
                {
                    fm.close_flow(false);
                }
                else
                {
                    fm.close_flow(true);
                }
            }

            Serial.println("Finished flow test!\nPrinting results...");
            for(int i = 0; i < highFlowLen; i++)
            {
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
    else
    {
        // connect to master device
        while(!greenLightFlag)
        {
            if(Serial.available() > 0)
            {
                rx_byte = Serial.read();

                if(rx_byte == '1')
                {
                    greenLightFlag = true;
                }
            }
        }

        // wait for system to be primed
        while(!readyState)
        {
            if(Serial.available() > 0)
            {
                rx_byte = Serial.read();

                switch(rx_byte)
                {
                    case '7':
                        // system is primed
                        readyState = true;
                        break;
                    case '1':
                        // turn on the pump
                        p_1.togglePump(true);
                        break;
                    case '0':
                        // turn off the pump
                        p_1.togglePump(false);
                        break;
                    default:
                        break;
                }
            }
        }        
        Serial.write("1");
        while(greenLightFlag && readyState)
        {
            // run flow manager 
            ByteCount = -1;
            ByteCount = Serial.readBytesUntil('\n', Buffer, sizeof(Buffer) - 1);

            if (ByteCount > 0) {
                // parse the buffer
                if(strstr(Buffer, ",")) { // if there is a comma in the buffer
                    strcpy(Command, strtok(Buffer, ",")); // copy first part as command
                    strcpy(Data, strtok(NULL, "\n")); // copy second part as data
                }
                else 
                { // only command sent through buffer
                    strcpy(Command, strtok(Buffer, "\n"));
                }
                if(!isspace(Command[0])) {
                    c = atoi(Command);
                    checkInput = true;
                }
                else {
                    checkInput = false;
                }
            }
            if(checkInput) 
            {
                switch (c) {
                    case 9:
                        // kill the operation

                        // close the flow
                        if(fm.flowAvg > 30.0)
                        {
                            fm.close_flow(false);
                        }
                        else
                        {
                            fm.close_flow(true);
                        }
                        
                        // turn off the pump
                        p_1.togglePump(false);

                        Serial.println("9");
                        break;
                    case 2:
                        temp = atoi(Data);

                        if(temp == 1)
                        {
                            p_1.togglePump(true);
                        }
                        else
                        {
                            p_1.togglePump(false);
                        }

                        Serial.println("2");
                        break;
                    case 1:
                        // new flow rate
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
    /*
    if(low_fs_active)
    {
        low_FS.set_liquid_type(isWater);
        delay(50);

        low_FS.readSensor();
        low_FS.scaleReadings();
        
        Serial.print("Flow rate value: ");
        Serial.print(low_FS.scaled_flow_value);
        Serial.print(" ml/min");
        Serial.println();
    
        Serial.print("Flags: ");
        Serial.print(low_FS.aux_value, BIN);
        Serial.println();
        
        Serial.print("Temperature value: ");
        Serial.print(low_FS.scaled_temp_value);
        Serial.print(" C");
        Serial.println();
        low_Motor.driveMotor();
    }
    else
    {
        high_FS.set_liquid_type(isWater);
        delay(50);

        high_FS.readSensor();
        high_FS.scaleReadings();
        
        Serial.print("Flow rate value: ");
        Serial.print(high_FS.scaled_flow_value);
        Serial.print(" ml/min");
        Serial.println();
        
        Serial.print("Flags: ");
        Serial.print(high_FS.aux_value, BIN);
        Serial.println();
        
        Serial.print("Temperature value: ");
        Serial.print(high_FS.scaled_temp_value);
        Serial.print(" C");
        Serial.println();
        high_Motor.driveMotor();
    }
    
    
    delay(50); // wait 50 milliseconds
    */
}

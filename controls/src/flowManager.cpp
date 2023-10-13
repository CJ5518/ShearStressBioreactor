#include "flowManager.hpp"

void flowManager::init()
{
    // initialize the i2c expander
    // low flow sensor is connected to slave device 0
    // high flow sensor is connected to slave device 1
    tca.init(0,1);

    low_Motor.init(9, 8, 10);
    high_Motor.init(6, 5, 7);
    tca.tcaselect(0);
    low_FS.init_sensor();
    tca.tcaselect(1);
    high_FS.init_sensor();
}

void flowManager::test_flow(float tgt_flowRate, bool isWater)
{
    bool lowFlowSys, flowInRange = false, openValve;
    float difference = 0.01, avg10Readings, temp;
    int motor_ticks, ticks_toDrive;

    // system checks before running motors
    Serial.print("Target flow rate: ");
    Serial.print(tgt_flowRate);
    Serial.println();

    if(tgt_flowRate == 0)
    {
        stepsTaken = 0;
    }

    // if the flow rate is larger than 30.0
    // then we use the high flow system
    if(tgt_flowRate > 30.0)
    {
        lowFlowSys = false;
        // error rate for high flow sensor is 10%
        if(tgt_flowRate > 300)
        {
            difference = tgt_flowRate * 0.05;
        }
        else 
        {
            difference = tgt_flowRate * 0.1;
        }
    }
    // else use the low flow system
    else 
    {
        lowFlowSys = true;
        // error rate for low flow sensor is 5%
        difference = tgt_flowRate * 0.05;
    } 

    // begin test run
    while(!flowInRange)
    {
        avg10Readings = 0;
        
        // average 10 flow readings then assign to current flow rate
        takeAvgNumReadings(lowFlowSys, 10);

        avg10Readings = flowAvg;

        /*
        for(int i = 0; i < 10; i++)
        {
            if(lowFlowSys)
            {
                tca.tcaselect(1);
                low_FS.set_liquid_type(isWater);
                delay(50);
                //Serial.println("low flow sensor");
                low_FS.readSensor();
                low_FS.scaleReadings();
                avg10Readings += low_FS.scaled_flow_value;
            }
            else 
            {
                tca.tcaselect(0);
                high_FS.set_liquid_type(isWater);
                delay(50);

                high_FS.readSensor();
                high_FS.scaleReadings();
                avg10Readings += high_FS.scaled_flow_value;
            }
            
        }
        */
        // get the average of the last ten readings
        //avg10Readings /= 10;
        
        Serial.print("Flow Rate: ");
        Serial.print(avg10Readings);
        Serial.print(" ml/min");
        Serial.println();

        // check if current flow rate is in range of target flow rate        
        if((avg10Readings > 0 && ((tgt_flowRate - avg10Readings) < difference && (avg10Readings - tgt_flowRate) < difference)) || tgt_flowRate == avg10Readings)
        {
            flowInRange = true;
            Serial.println("Flow rate in range!");
        }
        else 
        {
            temp = tgt_flowRate - avg10Readings;
            temp = (temp * temp) / 2;
        
        // adjust the motor based on range from target flow rate
            if(lowFlowSys)
            {
                // adjust low flow rate valve
                if(temp > 20)
                {
                    ticks_toDrive = 10000;
                }
                else if(temp > 10)
                {
                    ticks_toDrive = 8000;
                }
                /*else if(temp > 5)
                {
                    ticks_toDrive = 5000;
                }*/
                else if(temp > 2)
                {
                    ticks_toDrive = 5000;
                }
                else if(temp > 1.5)
                {
                    ticks_toDrive = 4000;
                }
                else if(temp > 1)
                {
                    ticks_toDrive = 3000;
                }
                else if(temp > 0.5)
                {
                    ticks_toDrive = 2000;
                }
                else if(temp > 0.20)
                {
                    ticks_toDrive = 1000;
                }
                else if(temp > 0.10)
                {
                    ticks_toDrive = 500;
                }
                else if(temp > 0.05)
                {
                    ticks_toDrive = 100;
                }
                else
                {
                    ticks_toDrive = 50;
                }
            }
            else 
            {
                // adjust high flow rate valve
                if(temp > 200)
                {
                    ticks_toDrive = 1000;
                }
                else if(temp > 100)
                {
                    ticks_toDrive = 500;
                }
                else if(temp > 10)
                {
                    ticks_toDrive = 200;
                }
                else if(temp > 5)
                {
                    ticks_toDrive = 100;
                }
                else if(temp > 2)
                {
                    ticks_toDrive = 50;
                }
                else if(temp > 1)
                {
                    ticks_toDrive = 20;
                }
                else if(temp > 0.5)
                {
                    ticks_toDrive = 5;
                }
                else
                {
                    ticks_toDrive = 1;
                }
            }
            // if the current flow rate is greater than the target flow rate
            if(avg10Readings > tgt_flowRate) 
            {
                // close the valve
                openValve = false;

                motor_ticks -= ticks_toDrive;
            }
            // the current flow rate is less than the target flow rate
            else
            {
                // open the valve
                openValve = true;

                motor_ticks += ticks_toDrive;
            }
            
            if(lowFlowSys)
            {
                low_Motor.driveMotor(ticks_toDrive, openValve);
            }
            else
            {
                high_Motor.driveMotor(ticks_toDrive, openValve);
            }

            // wait for flow rate to adjust
            delay(3000);
        }
    }
    stepsTaken = motor_ticks;
    Serial.print("motor ticks: ");
    Serial.print(motor_ticks);
    Serial.println();

    //return motor_ticks;
}

void flowManager::takeAvgNumReadings(bool lowFlow, int numReadings)
{
    float avg = 0.0;

    for(int i = 0; i < numReadings; i++)
    {
        if(lowFlow)
        {
            tca.tcaselect(1);
            low_FS.set_liquid_type(true);
            delay(50);
            
            low_FS.readSensor();
            low_FS.scaleReadings();

            // if the flow rate is 0, return 0
            /*
            if(low_FS.scaled_flow_value <= 0)
            {
                // flow rate is 0
                flowAvg = 0.0;
                // leave function
                return;
            }
            */
            avg += low_FS.scaled_flow_value;
        }
        else 
        {
            tca.tcaselect(0);
            high_FS.set_liquid_type(true);
            delay(50);

            high_FS.readSensor();
            high_FS.scaleReadings();

            // if the flow rate is 0, return 0
            /*
            if(high_FS.scaled_flow_value == 0)
            {
                // flow rate is 0
                flowAvg = 0;
                // leave function
                return;
            }
            */

            avg += high_FS.scaled_flow_value;
        }
    }
    flowAvg = avg / numReadings;
}

void flowManager::close_flow(bool lowFlow)
{
    while(flowAvg > 0.0)
    {
        takeAvgNumReadings(lowFlow, 10);
        Serial.println(flowAvg);

        //if(flowAvg < 1.0)

        if(flowAvg > 5.0)
        {
            if(lowFlow)
            {
                low_Motor.driveMotor(6400, false);
            }
            else
            {
                high_Motor.driveMotor(6400, false);
            } 
            //stepsTaken = stepsTaken / 2;
        }
        else
        {
            int tempSteps;

            if(flowAvg > 2.0)
            {
                tempSteps = 2000;
            }
            else if(flowAvg > 1.0)
            {
                tempSteps = 1000;
            }
            else if(flowAvg > 0.5)
            {
                tempSteps = 500;
            }
            else if(flowAvg > 0.2)
            {
                tempSteps = 500;
            }
            else
            {
                tempSteps = 200;
            }

            if(lowFlow)
            {
                low_Motor.driveMotor(tempSteps, false);
            }
            else
            {
                high_Motor.driveMotor(tempSteps, false);
            } 
            stepsTaken -= tempSteps;
        }
        
    }

    stepsTaken = 0;
}
#include "flowManager.hpp"

/*
 * Initialize the I2C expander, the stepper drivers and the sensors.
 */
void flowManager::init() {
    // Low flow sensor is connected to slave device 0
    // High flow sensor is connected to slave device 1
    tca.init(0, 1);

    // Low motor is driver 1 with smaller text: PUL (step): 12 DIR: 13 ENA: 14
    low_Motor.init(13, 12, 14); // dir, pul, ena
    // High motor is driver 2 with larger text: PUL (step): 26 DIR: 27 ENA: 25
    high_Motor.init(27, 26, 25);

    tca.tcaselect(0);
    low_FS.init_sensor();
    tca.tcaselect(1);
    high_FS.init_sensor();
}

/*
 * Opens the relevant valve until the target flow rate is achieved.
 */
void flowManager::test_flow(float tgt_flowRate, bool isWater) {
    bool lowFlowSys, flowInRange = false, openValve;
    float difference = 0.01, avg10Readings, temp;
    int motor_ticks, ticks_toDrive;

    // system checks before running motors
    Serial.print("Target flow rate: ");
    Serial.print(tgt_flowRate);
    Serial.println();

    if(tgt_flowRate == 0) {
        stepsTaken = 0;
    }

    // if the flow rate is larger than 30.0
    // then we use the high flow system
    if(tgt_flowRate > 30.0) {
        lowFlowSys = false;
        // error rate for high flow sensor is 10%
        if(tgt_flowRate > 300) {
            difference = tgt_flowRate * 0.05;
        }
        else {
            difference = tgt_flowRate * 0.1;
        }
    }
    // else use the low flow system
    else {
        lowFlowSys = true;
        // error rate for low flow sensor is 5%
        difference = tgt_flowRate * 0.05;
    } 

    // begin test run
    while(!flowInRange) {
        avg10Readings = 0;
        
        // average 10 flow readings then assign to current flow rate
        takeAvgNumReadings(lowFlowSys, 10);

        avg10Readings = flowAvg;
        
        Serial.print("Flow Rate: ");
        Serial.print(avg10Readings);
        Serial.print(" ml/min");
        Serial.println();

        // check if current flow rate is in range of target flow rate        
        if((avg10Readings > 0 && ((tgt_flowRate - avg10Readings) < difference && (avg10Readings - tgt_flowRate) < difference)) || tgt_flowRate == avg10Readings) {
            flowInRange = true;
            Serial.println("Flow rate in range!");
        }
        else {
            temp = tgt_flowRate - avg10Readings;
            temp = (temp * temp) / 2;
        
            // Adjust the motor based on range from target flow rate
            if(lowFlowSys) {
                // Adjust low flow rate valve
                if(temp > 20) {
                    ticks_toDrive = 10000;
                }
                else if(temp > 10) {
                    ticks_toDrive = 8000;
                }
                else if(temp > 2) {
                    ticks_toDrive = 5000;
                }
                else if(temp > 1.5) {
                    ticks_toDrive = 4000;
                }
                else if(temp > 1) {
                    ticks_toDrive = 3000;
                }
                else if(temp > 0.5) {
                    ticks_toDrive = 2000;
                }
                else if(temp > 0.20) {
                    ticks_toDrive = 1000;
                }
                else if(temp > 0.10) {
                    ticks_toDrive = 500;
                }
                else if(temp > 0.05) {
                    ticks_toDrive = 100;
                }
                else {
                    ticks_toDrive = 50;
                }
            }
            else {
                // Adjust high flow rate valve
                if(temp > 200) {
                    ticks_toDrive = 1000;
                }
                else if(temp > 100) {
                    ticks_toDrive = 500;
                }
                else if(temp > 10) {
                    ticks_toDrive = 200;
                }
                else if(temp > 5) {
                    ticks_toDrive = 100;
                }
                else if(temp > 2) {
                    ticks_toDrive = 50;
                }
                else if(temp > 1) {
                    ticks_toDrive = 20;
                }
                else if(temp > 0.5) {
                    ticks_toDrive = 5;
                }
                else {
                    ticks_toDrive = 1;
                }
            }

            // If the current flow rate is greater than the target flow rate
            if (avg10Readings > tgt_flowRate) {
                // Close the valve
                openValve = false;

                motor_ticks -= ticks_toDrive;
            }
            else {
                // Open the valve
                openValve = true;

                motor_ticks += ticks_toDrive;
            }
            
            // Step the relevant motor the number of ticks that was just determined
            if(lowFlowSys) {
                low_Motor.driveMotor(ticks_toDrive, openValve);
            }
            else {
                high_Motor.driveMotor(ticks_toDrive, openValve);
            }

            // Wait for flow rate to adjust
            delay(500);
        }
    }
    stepsTaken = motor_ticks;
    Serial.print("Motor ticks: ");
    Serial.print(motor_ticks);
    Serial.println();
}

/*
 * Reads from the specified flow sensor the requested number of times, and sets the global flowAvg to the average value.
 */
void flowManager::takeAvgNumReadings(bool lowFlow, int numReadings) {
    float avg = 0.0;

    // Loop the requested number of times
    for (int i = 0; i < numReadings; i++) {
        if (lowFlow) {
            tca.tcaselect(1);
            low_FS.set_liquid_type(true);
            delay(50);
            
            low_FS.readSensor();
            low_FS.scaleReadings();

            avg += low_FS.scaled_flow_value;
        }
        else {
            tca.tcaselect(0);
            high_FS.set_liquid_type(true);
            delay(50);

            high_FS.readSensor();
            high_FS.scaleReadings();

            avg += high_FS.scaled_flow_value;
        }
    }

    flowAvg = avg / numReadings;
}

/*
 * Closes the open valve until the measured flow is 0.
 */
void flowManager::close_flow(bool lowFlow) {
    // This may be problematic, since the pump stays on
    while(flowAvg > 0.0) {
        takeAvgNumReadings(lowFlow, 10);
        Serial.println(flowAvg);

        if(flowAvg > 5.0) {
            // Drive the relevant motor to close the valve
            if(lowFlow) {
                low_Motor.driveMotor(6400, false);
            }
            else {
                high_Motor.driveMotor(6400, false);
            }
        }
        else {
            int tempSteps;

            if(flowAvg > 2.0) {
                tempSteps = 2000;
            }
            else if(flowAvg > 1.0) {
                tempSteps = 1000;
            }
            else if(flowAvg > 0.5) {
                tempSteps = 500;
            }
            else if(flowAvg > 0.2) {
                tempSteps = 500;
            }
            else {
                tempSteps = 200;
            }

            if(lowFlow) {
                low_Motor.driveMotor(tempSteps, false);
            }
            else {
                high_Motor.driveMotor(tempSteps, false);
            } 
            stepsTaken -= tempSteps;
        }
    }

    stepsTaken = 0;
}
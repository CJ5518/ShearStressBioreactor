#include "flowManager.hpp"

/*
 * Creates a FlowManager object that will use the provided pump controller. I2C communication and steppers are also initialized.
 */
FlowManager::FlowManager(Pump* p) {
    pump = p;
    init();
}

/*
 * Initialize the I2C expander, the stepper drivers and the sensors.
 */
void FlowManager::init() {
    // Low flow sensor is connected as device 0
    // High flow sensor is connected as device 1
    tca.init(0, 1);

    /*pid.begin();
    pid.tune(0.1, 0.5, 0.3); // kP, kI, kD
    pid.limit(MIN_SPEED, MAX_SPEED);*/

    // Low motor is driver 1 with smaller text: PUL (step): 12 DIR: 13 ENA: 14
    lowMotor.init(13, 12, 14); // dir, pul, ena
    // High motor is driver 2 with larger text: PUL (step): 26 DIR: 27 ENA: 25
    highMotor.init(27, 26, 25);

    // Create flow sensor objects and send initial reset commands
    tca.tcaSelect(0);
    lowFS.initSensor();
    tca.tcaSelect(1);
    highFS.initSensor();
}

/*
 * Opens the relevant valve until the target flow rate is achieved.
 */
void FlowManager::setFlow(float targetFlow) {
    bool lowFlowSys; // whether the low valve should be used
    bool flowInRange = false; // will be obsolete once the loop is moved
    bool openValve; // whether the valve should be opened or closed
    float difference; // error bounds on target flow
    float avg10Readings; // current flow rate
    int motorTicks;

    // Print target
    Serial.print("Target flow rate: ");
    Serial.print(targetFlow);
    Serial.println();

    // Use the high flow valve and sensor for all flow rates over 30 ml/min
    if (targetFlow > 30.0) {
        lowFlowSys = false;

        // Accuracy is higher for flow rates below 300 ml/min
        if(targetFlow <= 300) {
            difference = targetFlow * 0.05;
        }
        else {
            difference = targetFlow * 0.1;
        }
    }
    else {
        lowFlowSys = true;
        // Error rate for low flow sensor is 5%
        difference = targetFlow * 0.05;
    }

    // Close the old valve and change the pump speed when switching between the two systems (low->high or high->low)
    if (targetFlow == 0 || lastSys != lowFlowSys) {
        closeFlow(lastSys);
        Serial.println("Setting flow rate to 0.");
        stepsTaken = 0;

        // TODO: consider setting the flow rate more precisely to avoid turbulence when clamping to flow rates at the lower limit of each system
        // If we are changing from a high flow rate to a low flow rate, set the pump speed to the max low flow rate
        if (lowFlowSys) {
            pump->setSpeed(MAX_LOW_FLOW_RATE);
        }
        else {
            pump->setSpeed(MAX_HIGH_FLOW_RATE); // set the pump speed to the max high flow rate
        }
    }

    // Save the valve that was adjusted last
    lastSys = lowFlowSys;

    // Repeat until the current flow rate is close to the target
    // TODO: remove the loop, call this function multiple times from main loop with interrupts
    while (!flowInRange) {
        avg10Readings = 0;
        
        // average 10 flow readings then assign to current flow rate
        avg10Readings = takeAvgNumReadings(lowFlowSys, 10);
        
        Serial.print("Flow Rate: ");
        Serial.print(avg10Readings);
        Serial.print(" ml/min");
        Serial.println();

        // Check if current flow rate is in range of target flow rate        
        if (avg10Readings > 0 && abs(targetFlow - avg10Readings) < difference) {
            flowInRange = true;
            Serial.println("Flow rate in range!");
        }
        else {
            // Set the target flow rate and provide current flow
            pid.setpoint(targetFlow);
            ticksToDrive = pid.compute(avg10Readings); // controller will return the ticks to drive the stepper

            // If the current flow rate is greater than the target flow rate
            if (avg10Readings > targetFlow) {
                // Close the valve
                openValve = false;
                motorTicks -= ticksToDrive;
            }
            else {
                // Open the valve
                openValve = true;
                motorTicks += ticksToDrive;
            }
            
            // Step the relevant motor the number of ticks that was just determined
            if (lowFlowSys) {
                lowMotor.driveMotor(ticksToDrive, openValve);
            }
            else {
                highMotor.driveMotor(ticksToDrive, openValve);
            }

            // Wait for flow rate to adjust
            delay(500); // TODO: update delay for new pump and replace with schedule call in main loop
        }
    }

    // Print the total number of ticks to reach the target
    stepsTaken = motorTicks;
    Serial.print("Motor ticks: ");
    Serial.print(motorTicks);
    Serial.println();
}

/*
 * Reads from the specified flow sensor the requested number of times, and returns the average value.
 */
float FlowManager::takeAvgNumReadings(bool lowFlow, int numReadings) {
    float avg = 0.0;
    float reading;

    // Select the requested sensor and set the liquid to water which starts reading
    tca.readSensor(lowFlow); // select requested sensor address
    
    // Loop the requested number of times
    for (int i = 0; i < numReadings; i++) {
        if (lowFlow) {
            lowFS.setLiquid(true);
        }
        else {
            highFS.setLiquid(true);
        }
        delay(12); // minimum reliable delay between selecting the sensor and reading from it

        if (lowFlow) {
            reading = lowFS.scaleReadings(lowFS.readSensor());
        }
        else {
            reading = highFS.scaleReadings(highFS.readSensor());
        }

        Serial.println(reading);
        avg += reading;
    }

    return avg / numReadings;
}

/*
 * Closes the open valve until the measured flow is 0.
 */
void FlowManager::closeFlow(bool lowFlow) {
    float flowAvg = takeAvgNumReadings(lowFlow, 10);

    // There must be a bypass line in order to close the valve while the pump is on!
    while(flowAvg > 0.0) {
        Serial.println(flowAvg);
        // Use the controller to determine how to close the valves
        pid.setpoint(0);
        int diffSteps = pid.compute(flowAvg);

        if(lowFlow) {
            lowMotor.driveMotor(diffSteps, false);
        }
        else {
            highMotor.driveMotor(diffSteps, false);
        }

        delay(200); // TODO: test for new pump and replace with scheduled call
        flowAvg = takeAvgNumReadings(lowFlow, 10);
    }

    Serial.print(lowFlow ? "Low" : "High");
    Serial.println(" flow rate valve closed.");
}
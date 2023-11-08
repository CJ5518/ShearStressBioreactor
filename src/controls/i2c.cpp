#include "i2c.hpp"

#include <Arduino.h>
#include <Wire.h>

void i2c::init() {
    Wire.begin();
}
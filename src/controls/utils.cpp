#include "utils.hpp"
#include <Arduino.h>

/*
 * Calculates the flow rate in ml/min required to achieve the provided shear stress in Pa.
 */
double Utils::flowRate(double tau) {
    return ((Utils::CHANNEL_WIDTH * Utils::CHANNEL_HEIGHT * Utils::CHANNEL_HEIGHT * tau) / (6 * Utils::MU)) * 6e7;
}

/*
 * Calculates the shear stress in Pa that will result from the provided flow rate in ml/min.
 */
double Utils::shearStress(double q) {
    return (q / 6e7) * (6 * Utils::MU) / (Utils::CHANNEL_WIDTH * Utils::CHANNEL_HEIGHT * Utils::CHANNEL_HEIGHT);
}

/*
 * Calculates the Reynolds number for the given flow rate.
 */
double Utils::reynolds(double q) {
    return (Utils::RHO * (q / (Utils::CHANNEL_HEIGHT * Utils::CHANNEL_WIDTH)) * Utils::CHANNEL_HEIGHT) / Utils::MU;
}
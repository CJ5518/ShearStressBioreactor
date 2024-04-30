/****************************************************************************************************
 * utils.cpp
 * Carson Sloan
 * 
 * Defines utility functions for calculating the Reynolds number for the flow through the bioreactor,
 * and transforming between flow rate and shear stress. The calculations are sent to the relevant
 * ThingSpeak fields automatically.
/****************************************************************************************************/

#include "utils.hpp"
#include "thingSpeak.hpp"

extern ThingSpeak tsp;

/*
 * Calculates the flow rate in ml/min required to achieve the provided shear stress in Pa.
 */
double Utils::flowRate(double tau) {
    double fl = ((Utils::CHANNEL_WIDTH * Utils::CHANNEL_HEIGHT * Utils::CHANNEL_HEIGHT * tau) / (6 * Utils::MU)) * 6e7;
    tsp.sendToThingSpeak_field3(fl);
    return fl;
}

/*
 * Calculates the shear stress in Pa that will result from the provided flow rate in ml/min.
 */
double Utils::shearStress(double q) {
    double sh = (q / 6e7) * (6 * Utils::MU) / (Utils::CHANNEL_WIDTH * Utils::CHANNEL_HEIGHT * Utils::CHANNEL_HEIGHT);
    tsp.sendToThingSpeak_field6(sh);
    return sh;
}

/*
 * Calculates the Reynolds number for the given flow rate.
 */
double Utils::reynolds(double q) {
    double r = (Utils::RHO * (q / (Utils::CHANNEL_HEIGHT * Utils::CHANNEL_WIDTH)) * Utils::CHANNEL_HEIGHT) / Utils::MU;
    tsp.sendToThingSpeak_field8(r);
    return r;
}
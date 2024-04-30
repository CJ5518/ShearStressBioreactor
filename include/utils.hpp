/****************************************************************************************************
 * utils.hpp
 * Carson Sloan
 * 
 * Declares the Utils class and specifies the phyisical bioreactor parameters.
/****************************************************************************************************/

#pragma once

class Utils {
    public:
        // Global static utility functions
        static double flowRate(double tau);
        static double shearStress(double tau);
        static double reynolds(double q);

    private:
        // If a new bioreactor design is used, replace these values with the new measurements
        static constexpr double CHANNEL_HEIGHT = 0.0005; // m
        static constexpr double CHANNEL_WIDTH = 0.04; // m

        static constexpr double MU = 0.0006922; // Pa * s
        static constexpr double RHO = 993; // kg / m^3
};
 
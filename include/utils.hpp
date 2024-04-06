#pragma once

class Utils {
    public:
        // Global static utility functions
        double flowRate(double tau);
        double shearStress(double tau);
        double reynolds(double q);

    private:
        const double CHANNEL_HEIGHT = 0.0005; // m
        const double CHANNEL_WIDTH = 0.04; // m
        const double MU = 0.0006922; // Pa * s
        const double RHO = 993; // kg / m^3
};
 
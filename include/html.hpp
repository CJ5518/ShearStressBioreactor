#ifndef HTML_HPP
#define HTML_HPP

//This file contains various files for the web server, not just html but also css and js
//Doing this was so much easier than SPIFFS what can I say

#include <Arduino.h>


const char index_html[] PROGMEM = "<!DOCTYPE html><html><head><title>ESP 32</title></head><body><h1>Connected!</h1><p>You have successfully connected to the ESP32.</p></body></html>";

#endif

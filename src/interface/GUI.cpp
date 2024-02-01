//Implements GUI.hpp
#include "GUI.hpp"

//#include "SPIFFS.h"
#include "WiFi.h"

//Credentials for the Access point
//In the future we'd like to be able to save/change/load these
//But for now the const variables work fine
const char* ssid     = "AAShowUpPlz_26";
const char* password = "password1234";


//Empty constructor, just call init bro
GUI::GUI () {}

GUI::GUI(RoutineManager* rm) {
    init(rm);
}


void GUI::init(RoutineManager* rm) {
    routineManager = rm;


	// Initialize SPIFFS
	//if(!SPIFFS.begin()){
	//	Serial.println("An Error has occurred while mounting SPIFFS");
	//	return;
	//}
    delay(1000);

    WiFi.mode(WIFI_AP); 
    Serial.println("Past mode");
    delay(300);
    WiFi.softAP(ssid, password);
    delay(300);
    Serial.println("Past delay");
    IPAddress Ip(192, 168, 4, 44);
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);
    // Show IP address.
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("\nESP32 AP IP address: ");
    Serial.println(myIP);

    //I've dealt with enough issues
    //We're waiting a few seconds now, better than the thing not working at all
    delay(2000);
    server = new AsyncWebServer(80);
    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        //request->send(SPIFFS, "/index.html");
        request->send(200, "text/plain","Hello, World!");
    });
    server->begin();
    Serial.println("Finished GUI init");
}

void GUI::end() {
    delete server;
}

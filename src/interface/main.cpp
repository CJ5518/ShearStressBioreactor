
#include "SPIFFS.h"
#include "WiFi.h"
#include "ESPAsyncWebServer.h"


// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "password";

// Set web server port number to 80
AsyncWebServer server(80);

//https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/

void setupOLD() {
	Serial.begin(115200);
  
	// Initialize SPIFFS
	if(!SPIFFS.begin(true)){
		Serial.println("An Error has occurred while mounting SPIFFS");
		return;
	}

	// Connect to Wi-Fi network with SSID and password
	Serial.print("Setting AP (Access Point)…");
	// Remove the password parameter, if you want the AP (Access Point) to be open
	WiFi.softAP(ssid, password);

	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);
	

	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(SPIFFS, "/index.html");
	});
  
  server.begin();
}


void loopOLD(){

}

//Implements GUI.hpp
#include "GUI.hpp"

#include "WiFi.h"
#include "html.hpp"

//https://web.archive.org/web/20240213064921/https://gist.githubusercontent.com/evert-arias/
//d0abf2769802e56c88793a4447fe9f7e/raw/b1f582f8cdcaa07f84072ede8687bdbe9045e75a/esp32-wifi-auto-connect.cpp

//Credentials for the Access point
//In the future we'd like to be able to save/change/load these
//But for now the const variables work fine
const char* ssid     = "ESP32 Bioreactor";
const char* password = "password1234";

IPAddress apIP = IPAddress(192,168,4,42);

#define WIFI_CONNECT_INTERVAL 5000   // Connection retry interval in milliseconds
#define WIFI_WATCHDOG_INTERVAL 5000  // Wi-Fi watchdog interval in milliseconds


RoutineManager* GUI::routineManager;
AsyncWebServer* GUI::server = 0;
Task* GUI::wifi_connect_task;
Task* GUI::wifi_watchdog_task;
// Wi-Fi events
//Default wifi event definitions are WRONG because everybody hates me I guess

/*
[  5140][V][WiFiGeneric.cpp:392] _arduino_event_cb(): AP Started
[  5141][D][WiFiGeneric.cpp:1035] _eventCallback(): Arduino Event: 10 - AP_START


On user connect (2 events)
[ 27801][V][WiFiGeneric.cpp:407] _arduino_event_cb(): AP Station Connected: MAC: 4a:84:27:1a:f7:15, AID: 1
[ 27802][D][WiFiGeneric.cpp:1035] _eventCallback(): Arduino Event: 12 - AP_STACONNECTED

[ 28327][V][WiFiGeneric.cpp:421] _arduino_event_cb(): AP Station IP Assigned:192.168.4.2
[ 28328][D][WiFiGeneric.cpp:1035] _eventCallback(): Arduino Event: 14 - AP_STAIPASSIGNED

On user disconnect (1 event)
[ 33674][V][WiFiGeneric.cpp:414] _arduino_event_cb(): AP Station Disconnected: MAC: 4a:84:27:1a:f7:15, AID: 1
[ 33675][D][WiFiGeneric.cpp:1035] _eventCallback(): Arduino Event: 13 - AP_STADISCONNECTED
*/




void GUI::onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
  Serial.printf("Got event %d - %s\n", event, WiFi.eventName(event));
  switch (event) {
    case 10: {
        Serial.printf("AP Started\n");
        WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
        if (!server) {
            server = new AsyncWebServer(80);

            server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
                request->send_P(200, "text/html", index_html);
            });

            server->begin();
            Serial.printf("Started server\n");
        }

    } break;
    
  }
}

// Wi-Fi connect task
 void GUI::wifi_connect_cb() {
    // Disable this task to avoid further iterations
    wifi_connect_task->disable();

    // Connect to Wi-Fi network
    bool ret = WiFi.enableAP(true);
    ret = WiFi.softAP(ssid, password);

    wifi_watchdog_task->enable();
}

//Wi-Fi watchdog task
//Mostly used for other wifi modes
void GUI::wifi_watchdog_cb() {
}

void GUI::init(Scheduler* ts, RoutineManager* rm) {
    routineManager = rm;
    Serial.printf("In GUI init\n");

    static Task wifiConnectTask(WIFI_CONNECT_INTERVAL, TASK_FOREVER, &wifi_connect_cb);
    static Task wifiWatchdogTask(WIFI_WATCHDOG_INTERVAL, TASK_FOREVER, &wifi_watchdog_cb);
    
    ts->addTask(wifiConnectTask);
    ts->addTask(wifiWatchdogTask);

    // Wi-Fi events listener
    WiFi.onEvent(onWifiEvent);

    // Wait and enable wifi connect task
    wifiConnectTask.enableDelayed(5000);
    //Ran into some heap issues so we do it like this
    //Which is why the tasks are static
    wifi_connect_task = &wifiConnectTask;
    wifi_watchdog_task = &wifiWatchdogTask;
}

void GUI::end() {
    delete server;
    delete wifi_connect_task;
    delete wifi_watchdog_task;
}

//Implements GUI.hpp
#include "GUI.hpp"

#include "WiFi.h"
#include "html.hpp"

#include "Preferences.h"

//https://web.archive.org/web/20240213064921/https://gist.githubusercontent.com/evert-arias/
//d0abf2769802e56c88793a4447fe9f7e/raw/b1f582f8cdcaa07f84072ede8687bdbe9045e75a/esp32-wifi-auto-connect.cpp

//Credentials for the Access point
//In the future we'd like to be able to save/change/load these
//But for now the const variables work fine
//const char* ssid     = "ESP32 Bioreactor";
//const char* password = "password1234";

IPAddress apIP = IPAddress(192,168,4,42);

#define WIFI_CONNECT_INTERVAL 5000   // Connection retry interval in milliseconds
#define WIFI_WATCHDOG_INTERVAL 5000  // Wi-Fi watchdog interval in milliseconds

Preferences guiPreferences;

RoutineManager* GUI::routineManager;
Scheduler* GUI::taskScheduler;
AsyncWebServer* GUI::server = 0;
static int errorCode;
static String errorMessage;
static String successMessage;
static Task* wifi_connect_task;
static Task* wifi_watchdog_task;
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


String errorMessageProcessor(const String& var) {
    if (var == "CODE") {
        return String(errorCode);
    } else if (var == "MESSAGE") {
        return errorMessage;
    }
    return String();
}

String successMessageProcessor(const String& var) {
    if (var == "MESSAGE") {
        return successMessage;
    }
    return String();
}

void sendErrorMessage(AsyncWebServerRequest* request, int code, const String &message) {
    errorCode = code;
    errorMessage = message;
    request->send_P(code, "text/html", error_html, errorMessageProcessor);
}

void sendSuccessMessage(AsyncWebServerRequest* request, const String& message) {
    successMessage = message;
    request->send_P(200, "text/html", success_html, successMessageProcessor);
}

void GUI::initServer(AsyncWebServer* server) {


    //Routine manager
    server->on("/executeRoutine", HTTP_POST, [](AsyncWebServerRequest* request) {
        Event* head = 0;
        for (int q = 0; q < request->getParam("numEntries", true)->value().toInt(); q++) {
            String base = ("entry" + String(q));
            Event* newEvent = new Event(
                request->getParam(base + "0", true)->value().toFloat(),
                (int)request->getParam(base + "1", true)->value().toFloat(),
                (int)request->getParam(base + "3", true)->value().toFloat(),
                (int)request->getParam(base + "2", true)->value().toFloat()
            );
            Serial.printf("Event %d: Flow: %f, timeOn: %d, timeOff: %d, cycles: %d\n", q,
                newEvent->getFlow(),
                newEvent->getDuration(),
                newEvent->getOffDuration(),
                newEvent->getRepetitions()
            );
            if (head == 0) {
                head = newEvent;
            } else {
                head->append(newEvent);
            }
        }

        routineManager->run(head);
    });


    //Network settings
    server->on("/networkSettingsRouterConnection", HTTP_POST, [](AsyncWebServerRequest *request) {
        //HTML forms are apparently terrible and don't include checkboxes unless they are checked
        //The check box is NOT checked
        Serial.println("NON AP settings");
        guiPreferences.begin("gui", false);
        if (request->params() == 2) {
        } else if (request->params() == 3) {
            //Check box IS checked
            Serial.println("Setting routerConnect to true!");
            guiPreferences.putBool("routerConnect", true);
            wifi_connect_task->enableDelayed(4000);
        } else {
            //If we get some other number, then something is seriously wrong
            sendErrorMessage(request, 500, "Internal server error, too many params at line " + String(__LINE__));
            return;
        }

        for (int q = 0; q < request->params(); q++) {
            Serial.printf("%d: '%s'='%s'\n", q, request->getParam(q)->name(), request->getParam(q)->value());
            Serial.printf("%d: '%s'='%s'\n", q, request->getParam(q)->name().c_str(), request->getParam(q)->value().c_str());
        }

        //Save other preferences
        guiPreferences.putString("ssid", request->getParam("ssid", true, false)->value().c_str());
        guiPreferences.putString("password", request->getParam("password", true, false)->value().c_str());
        guiPreferences.end();
        guiPreferences.begin("gui", true);
        char buff[100];
        char buff2[100];
        guiPreferences.getString("ssid", buff, 100);
        guiPreferences.getString("password", buff2, 100);
        Serial.printf("'%s', '%s'", buff, buff2);
        guiPreferences.end();
    });

    //Network settings AP
    server->on("/networkSettingsAccessPoint", HTTP_POST, [](AsyncWebServerRequest *request) {
        //HTML forms are apparently terrible and don't include checkboxes unless they are checked
        //The check box is NOT checked
        Serial.println("AP settings");
        guiPreferences.begin("gui", false);
        if (request->params() == 2) {
        } else if (request->params() == 3) {
            //Check box IS checked
            guiPreferences.putBool("routerConnect", false);
            wifi_connect_task->enableDelayed(4000);
        } else {
            //If we get some other number, then something is seriously wrong
            sendErrorMessage(request, 500, "Internal server error, too many params at line " + String(__LINE__));
            return;
        }
        //Save other preferences
        guiPreferences.putString("ssid", request->getParam("ssid", true, false)->value().c_str());
        guiPreferences.putString("password", request->getParam("password", true, false)->value().c_str());
        guiPreferences.end();
    });


    //Auto generated section, serves files in data/
    server->on("/common.js", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/javascript", common_js);
    });
    server->on("/error.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", error_html);
    });
    server->on("/help.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", help_html);
    });
    server->on("/index.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });
    server->on("/networkSettings.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", networkSettings_html);
    });
    server->on("/routineManager.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", routineManager_html);
    });
    server->on("/routineManager.js", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/javascript", routineManager_js);
    });
    server->on("/styles.css", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/css", styles_css);
    });
    server->on("/success.html", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", success_html);
    });
    server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {
        request->redirect("/index.html");
    });


    //Begin the server
    server->begin();
    Serial.printf("Started server\n");
}


void GUI::onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
    Serial.printf("Got event %d - %s\n", event, WiFi.eventName(event));
    switch (event) {
        case 10: {
            Serial.printf("AP Starting\n");
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

            delay(800);
            guiPreferences.begin("gui", true);
            WiFi.softAP(guiPreferences.getString("APssid"), guiPreferences.getString("APpassword"));
            guiPreferences.end();
            if (!server) {
                server = new AsyncWebServer(80);
                initServer(server);
            }
        } break; 
        case WIFI_EVENT_WIFI_READY: {
            
        } break;
    }
}

// Wi-Fi connect task
void wifi_connect_cb() {
    // Disable this task to avoid further iterations
    wifi_connect_task->disable();
    WiFi.disconnect();

    
    // Connect to Wi-Fi network
    guiPreferences.begin("gui", false);
    if (guiPreferences.getBool("routerConnect")) {
        Serial.printf("Trying to connect to router");
        WiFi.enableAP(false);
        //WiFi.config(apIP, apIP, IPAddress(255,255,255,0));
        char buff[100];
        char buff2[100];
        guiPreferences.getString("ssid", buff, 100);
        guiPreferences.getString("password", buff2, 100);
        Serial.printf("'%s', '%s'", buff, buff2);
        WiFi.begin(buff, buff2);

        // Wait for connection result and capture the result code
        uint8_t result = WiFi.waitForConnectResult();

        // Serial.printf("[Wi-Fi] Connection Result: %d \n", result);

        if (result == WL_NO_SSID_AVAIL || result == WL_CONNECT_FAILED)
        {
            // Fail to connect or SSID no available
            Serial.println(PSTR("[Wi-Fi] Status: Could not connect to Wi-Fi AP"));

            guiPreferences.putBool("routerConnect", false);
            // Wait and reenable this task to keep trying to connect
            wifi_connect_task->enableDelayed(1000);
        }
        else if (result == WL_IDLE_STATUS)
        {
            // Wi-Fi Idle. This means that it's connected to the AP but the DHCP has not assigned an IP yet
            Serial.println(PSTR("[Wi-Fi] Status: Idle | No IP assigned by DHCP Server"));
        }
        else if (result == WL_CONNECTED)
        {
            // Wi-Fi Connected
            Serial.printf(PSTR("[Wi-Fi] Status: Connected | IP: %s\n"), WiFi.localIP().toString().c_str());
        }


    } else {
        Serial.printf("Making access point");
        bool ret = WiFi.enableAP(true);
        ret = WiFi.softAP(guiPreferences.getString("APssid"), guiPreferences.getString("APpassword"));
    }
    guiPreferences.end();
    wifi_watchdog_task->enable();
}

//Wi-Fi watchdog task
//Mostly used for other wifi modes
void wifi_watchdog_cb() {
    Serial.printf("Status: %d, conn: %d\n", WiFi.status(), WL_CONNECTED);
}

void GUI::init(Scheduler* ts, RoutineManager* rm) {
    routineManager = rm;
    Serial.printf("In GUI init\n");

    static Task wifiConnectTask(WIFI_CONNECT_INTERVAL, TASK_FOREVER, &wifi_connect_cb);
    static Task wifiWatchdogTask(WIFI_WATCHDOG_INTERVAL, TASK_FOREVER, &wifi_watchdog_cb);
    
    ts->addTask(wifiConnectTask);
    ts->addTask(wifiWatchdogTask);

    taskScheduler = ts;

    // Wi-Fi events listener
    WiFi.onEvent(onWifiEvent);

    // Wait and enable wifi connect task
    wifiConnectTask.enableDelayed(5000);
    //Ran into some heap issues so we do it like this
    //Which is why the tasks are static
    wifi_connect_task = &wifiConnectTask;
    wifi_watchdog_task = &wifiWatchdogTask;

    guiPreferences.begin("gui", false);
    //If NONE of the preferences have been set
    if (!guiPreferences.isKey("ssid")) {
        //Go ahead and make them all
        guiPreferences.putString("ssid", "NONE");
        guiPreferences.putString("password", "NONE");
        guiPreferences.putString("APssid", "ESP32 Bioreactor");
        guiPreferences.putString("APpassword", "password1234");
        guiPreferences.putBool("routerConnect", false);
    }
    guiPreferences.end();
}

void GUI::end() {
    delete server;
    delete wifi_connect_task;
    delete wifi_watchdog_task;
}

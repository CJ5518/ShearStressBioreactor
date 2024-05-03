//Implements GUI.hpp (Carson Rueber)
#include "GUI.hpp"

#include "WiFi.h"
#include "html.hpp"

#include "Preferences.h"

//This code contains adaptions from:
//https://web.archive.org/web/20240213064921/https://gist.githubusercontent.com/evert-arias/
//d0abf2769802e56c88793a4447fe9f7e/raw/b1f582f8cdcaa07f84072ede8687bdbe9045e75a/esp32-wifi-auto-connect.cpp

//Credentials for the Access point
const char* ssid     = "ESP32 Bioreactor";
const char* password = "password1234";

//The access point IP Address
IPAddress apIP = IPAddress(192,168,4,42);

//Task intervals
#define WIFI_CONNECT_INTERVAL 5000   // Connection retry interval in milliseconds
#define WIFI_WATCHDOG_INTERVAL 5000  // Wi-Fi watchdog interval in milliseconds

//Static class members
RoutineManager* GUI::routineManager;
Scheduler* GUI::taskScheduler;
AsyncWebServer* GUI::server = 0;

//Global Variables
static int errorCode;
static String errorMessage;
static String successMessage;
static bool routerConnect = false;
static String ssidSTA = "";
static String passwordSTA = "";

//Callback method prototypes
void wifi_connect_cb();
void wifi_watchdog_cb();
bool enable_print();

//Tasks
Task wifi_connect_task(0, 1, &wifi_connect_cb);
Task wifi_watchdog_task(WIFI_WATCHDOG_INTERVAL, TASK_FOREVER, &wifi_watchdog_cb);
static Task test(0, 1, &wifi_connect_cb);

//Handle error/success messages
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

//Helper functions to send error/success messages
//Didn't end up being used too often in the final version
void sendErrorMessage(AsyncWebServerRequest* request, int code, const String &message) {
    errorCode = code;
    errorMessage = message;
    request->send_P(code, "text/html", error_html, errorMessageProcessor);
}

void sendSuccessMessage(AsyncWebServerRequest* request, const String& message) {
    successMessage = message;
    request->send_P(200, "text/html", success_html, successMessageProcessor);
}

//Adds server routes
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
        if (request->params() == 2) {
        } else if (request->params() == 3) {
            //Check box IS checked
            Serial.println("Setting routerConnect to true!");
            routerConnect = true;
            test.enable();
        } else {
            //If we get some other number, then something is seriously wrong
            sendErrorMessage(request, 500, "Internal server error, too many params at line " + String(__LINE__));
            return;
        }

        //Save other preferences
        ssidSTA = request->getParam("ssid", true, false)->value().c_str();
        passwordSTA = request->getParam("password", true, false)->value().c_str();
    });

    //Network settings AP
    server->on("/networkSettingsAccessPoint", HTTP_POST, [](AsyncWebServerRequest *request) {
        //HTML forms are apparently terrible and don't include checkboxes unless they are checked
        //The check box is NOT checked
        Serial.println("AP settings");
        if (request->params() == 0) {
        } else if (request->params() == 1) {
            //Check box IS checked
            routerConnect = false;
        } else {
            //If we get some other number, then something is seriously wrong
            sendErrorMessage(request, 500, "Internal server error, too many params at line " + String(__LINE__));
            return;
        }
    });

    //Is a routine running? Clients want to know
    server->on("/isRoutineRunning", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", routineManager->isRunning() ? "yes" : "no");
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

// Wi-Fi events
//Default wifi event definitions are WRONG so we hardcode the number 10 instead
void GUI::onWifiEvent(arduino_event_id_t event, arduino_event_info_t info) {
    Serial.printf("Got event %d - %s\n", event, WiFi.eventName(event));
    switch (event) {
        case 10: {
            Serial.printf("AP Starting\n");
            //Set our custom IP address
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

            delay(800);
            WiFi.softAP(ssid, password);
            if (!server) {
                server = new AsyncWebServer(80);
                initServer(server);
            }
        } break;
    }
}

// Wi-Fi connect task
void wifi_connect_cb() {
    // Disable this task to avoid further iterations
    //wifi_connect_task.disable();
    WiFi.disconnect();

    // Connect to Wi-Fi network
    if (routerConnect) {
        Serial.println("Trying to connect to router");
        WiFi.enableAP(false);
        //WiFi.config(apIP, apIP, IPAddress(255,255,255,0));
        WiFi.begin(ssidSTA, passwordSTA);

        // Wait for connection result and capture the result code
        uint8_t result = WiFi.waitForConnectResult();

        // Serial.printf("[Wi-Fi] Connection Result: %d \n", result);

        if (result == WL_NO_SSID_AVAIL || result == WL_CONNECT_FAILED)
        {
            // Fail to connect or SSID no available
            Serial.println(PSTR("[Wi-Fi] Status: Could not connect to Wi-Fi AP"));

            routerConnect = false;
            // Wait and reenable this task to keep trying to connect
            wifi_connect_task.enableDelayed(1000);
            wifi_connect_task.setIterations(1);
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
        Serial.println("Making access point");
        bool ret = WiFi.enableAP(true);
        ret = WiFi.softAP(ssid, password);
    }
}

//Wi-Fi watchdog task
//Mostly used for other wifi modes
void wifi_watchdog_cb() {
    Serial.printf("Status: %d, conn: %d\n", WiFi.status(), WL_CONNECTED);
}

//Initialize things in setup()
void GUI::init(Scheduler* ts, RoutineManager* rm) {
    routineManager = rm;
    Serial.printf("In GUI init\n");

    taskScheduler = ts;
    taskScheduler->addTask(wifi_connect_task);
    taskScheduler->addTask(wifi_watchdog_task);
    taskScheduler->addTask(test);
    //test = new Task(0, 1, &wifi_connect_cb, taskScheduler, false);

    // Wi-Fi events listener
    WiFi.onEvent(onWifiEvent);

    // Wait and enable wifi connect task
    wifi_connect_task.enableDelayed(5000);
    //wifi_watchdog_task.enable();
}

//Clean up
void GUI::end() {
    delete server;
}

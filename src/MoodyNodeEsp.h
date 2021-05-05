#include <EspWifiManager.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include <type_traits>

#define WEB_SERVER_PORT    80
#define MAX_PAYLOAD_LEN    8
#define MAX_JSON_CONN_SIZE 100
#define JSON_SIZE          64


template<typename T>
using SensorCallback = T (*)();

using ActuatorCallback = void (*)(String);


/*
    Sensor/Actuator APIs
*/

template<typename T>
class MoodySensor {
    private:
        AsyncWebServer sensorServer;
        SensorCallback<T> sensorCallback;

    public:
        MoodySensor();
        void setAcquireFunction(SensorCallback<T> callback);
        void begin();
        void loop();
        virtual ~MoodySensor() {}
};


class MoodyActuator {
    private:
        AsyncWebServer actuatorServer;
        ActuatorCallback actuatorCallback;

    public:
        MoodyActuator();
        void setActuateFunction(ActuatorCallback callback);
        void begin();
        void loop();
        virtual ~MoodyActuator() {}
};


/*
    Sensor implementation
*/


template<typename T>
MoodySensor<T>::MoodySensor() : sensorServer(WEB_SERVER_PORT), sensorCallback(nullptr)
{
    static_assert(std::is_integral<T>::value || std::is_same<T, String>::value || std::is_same<T, std::string>::value);
}


template<typename T>
void MoodySensor<T>::setAcquireFunction(SensorCallback<T> callback)
{
    sensorCallback = callback;
}


template<typename T>
void MoodySensor<T>::begin()
{
    sensorServer.on("/api/conn", HTTP_GET, [](AsyncWebServerRequest *request) {
        String resp;
        String mac = WiFi.macAddress();
        StaticJsonDocument<MAX_JSON_CONN_SIZE> jsonDoc;

        jsonDoc["type"] = "sensor";
        jsonDoc["mac"] = mac;
        serializeJson(jsonDoc, resp);

        request->send(200, "application/json", resp);
    });

    sensorServer.on("/api/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        StaticJsonDocument<JSON_SIZE> jsonDoc;
        if(this->sensorCallback)
        {
            T sensorData = this->sensorCallback();
            // if(sizeof(sensorData) > MAX_PAYLOAD_LEN)
            jsonDoc["payload"] = sensorData;
        }
        else
        {
            jsonDoc["payload"] = nullptr;
        }
        serializeJson(jsonDoc, resp);
        request->send(200, "application/json", resp);
    });

    sensorServer.begin();
}


template<typename T>
void MoodySensor<T>::loop()
{
    WiFiManager.loop();
}


/*
    Actuator implementation
*/

MoodyActuator::MoodyActuator() : actuatorServer(WEB_SERVER_PORT), actuatorCallback(nullptr)
{
}


void MoodyActuator::setActuateFunction(ActuatorCallback callback)
{
    actuatorCallback = callback;
}


void MoodyActuator::begin()
{
    actuatorServer.on("/api/conn", HTTP_GET, [](AsyncWebServerRequest *request) {
        String resp;
        String mac = WiFi.macAddress();
        StaticJsonDocument<MAX_JSON_CONN_SIZE> jsonDoc;

        jsonDoc["type"] = "actuator";
        jsonDoc["mac"] = mac;
        serializeJson(jsonDoc, resp);

        request->send(200, "application/json", resp);
    });

    actuatorServer.on("/api/data", HTTP_PUT, [this](AsyncWebServerRequest *request) {
        // deserialize and actuate, check for string type of payload
    });

    actuatorServer.begin();
}


void MoodyActuator::loop()
{
    WiFiManager.loop();
}

#include <EspWifiManager.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

#include <ArxTypeTraits.h>

#define WEB_SERVER_PORT    80
#define MAX_PAYLOAD_LEN    8
#define MAX_JSON_CONN_SIZE 150
#define JSON_SIZE          64
#define MAX_SERVICE_LEN    16
#define MAX_STRMSG_SIZE    8


template<typename T, unsigned int S = MAX_STRMSG_SIZE>
using SensorCallback = T (*)();

using ActuatorCallb ack = void (*)(String);


/*
    Sensor/Actuator APIs
*/

template<typename T>
class MoodySensor {
    private:
        const char* serviceName;
        AsyncWebServer sensorServer;
        SensorCallback<T, S> sensorCallback;

    public:
        MoodySensor(String serviceName);
        MoodySensor(const char* serviceName);
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


template<typename T, unsigned int S>
MoodySensor<T, S>::MoodySensor(String serviceName) : sensorServer(WEB_SERVER_PORT), sensorCallback(nullptr), serviceName(serviceName.c_str())
{
    static_assert(std::is_arithmetic<T>::value && && S > 0 && S <= MAX_STRMSG_SIZE && char[MAX_STRMSG_SIZE]);
}


template<typename T, int S>
MoodySensor<T, S>::MoodySensor(const char* serviceName) : sensorServer(WEB_SERVER_PORT), sensorCallback(nullptr), serviceName(serviceName)
{
    static_assert(std::is_arithmetic<T>::value && && S > 0 && S <= MAX_STRMSG_SIZE && char[MAX_STRMSG_SIZE]);
}


template<typename T, unsigned int S>
void MoodySensor<T, S>::setAcquireFunction(SensorCallback<T> callback)
{
    sensorCallback = callback;
}


template<typename T, unsigned int S>
void MoodySensor<T, S>::begin()
{
    sensorServer.on("/api/conn", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        String mac = WiFi.macAddress();
        StaticJsonDocument<MAX_JSON_CONN_SIZE> jsonDoc;

        jsonDoc["type"] = "sensor";
        jsonDoc["service"] = serviceName;
        jsonDoc["mac"] = mac;
        serializeJson(jsonDoc, resp);

        request->send(200, "application/json", resp);
    });

    sensorServer.on("/api/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        StaticJsonDocument<JSON_SIZE + S> jsonDoc;
        if(this->sensorCallback)
        {
            T sensorData = this->sensorCallback();
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


template<typename T, unsigned int S>
void MoodySensor<T, S>::loop()
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

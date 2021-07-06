#include <ssdpAWS.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ArxTypeTraits.h>
#include <EspWifiManager.h>
#include <ESPAsyncWebServer.h>


#define WEB_SERVER_PORT    80
#define MAX_JSON_CONN_SIZE 150
#define JSON_SIZE          64


template<typename T>
using SensorCallback = T (*)();

template<typename T>
using ActuatorCallback = void (*)(T);

static char chipId[4];
static const char modelName[] = "ESP8266";
static const char modelNumber[] = "v0.0.1";
static const char manufacturer[] = "Espressif";
static const char manufacturerUrl[] = "https://www.espressif.com/";

/*
    Sensor/Actuator APIs
*/

class MoodyBase {
    private:
        bool serverStarted = false;

    protected:
        AsyncWebServer webServer{WEB_SERVER_PORT};
        ssdpAWS ssdp{&webServer};
        const char* serviceName;

        explicit MoodyBase(const char* serviceName);
        virtual void begin() = 0;
    
    public:
        void loop();
        virtual ~MoodyBase() = default;
};


template<typename T>
class MoodySensor: public MoodyBase {
    private:
        SensorCallback<T> sensorCallback;

    public:
        explicit MoodySensor(const char* serviceName);
        void setAcquireFunction(SensorCallback<T> callback);
        void begin() override;
        ~MoodySensor() override = default;
};


template<typename T>
class MoodyActuator: public MoodyBase {
    private:
        T state;
        ActuatorCallback<T> actuatorCallback;

    public:
        explicit MoodyActuator(const char* serviceName);
        void setActuateFunction(ActuatorCallback<T> callback);
        void begin() override;
        ~MoodyActuator() override = default;
};


/*
    Base implementation
*/

MoodyBase::MoodyBase(const char* serviceName) : serviceName(serviceName) {}


void MoodyBase::loop()
{
    WiFiManager.loop();
    if(WiFiManager.isConnected())
    {
        if(!serverStarted)
        {
            snprintf(chipId, sizeof(chipId), "%u", ESP.getChipId());
            ssdp.begin(serviceName, chipId, modelName, modelNumber, manufacturer, manufacturerUrl);
            serverStarted = true;
            webServer.begin();
        }
    }
    else
    {
        if(serverStarted)
        {
            webServer.end();
        }
    }
}


/*
    Sensor implementation
*/


template<typename T>
MoodySensor<T>::MoodySensor(const char* serviceName) : MoodyBase(serviceName), sensorCallback(nullptr)
{
    static_assert(std::is_arithmetic<T>::value, "T can only be a number");
}


template<typename T>
void MoodySensor<T>::setAcquireFunction(SensorCallback<T> callback)
{
    sensorCallback = callback;
}


template<typename T>
void MoodySensor<T>::begin()
{
    webServer.on("/api/conn", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        String mac = WiFi.macAddress();
        StaticJsonDocument<MAX_JSON_CONN_SIZE> jsonDoc;

        jsonDoc["type"] = "sensor";
        jsonDoc["service"] = serviceName;
        jsonDoc["mac"] = mac;
        serializeJson(jsonDoc, resp);

        request->send(200, "application/json", resp);
    });

    webServer.on("/api/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        StaticJsonDocument<JSON_SIZE> jsonDoc;
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
}


/*
    Actuator implementation
*/


template<typename T>
MoodyActuator<T>::MoodyActuator(const char* serviceName) : MoodyBase(serviceName), actuatorCallback(nullptr)
{
    static_assert(std::is_arithmetic<T>::value, "T can only be a number");
}


template<typename T>
void MoodyActuator<T>::setActuateFunction(ActuatorCallback<T> callback)
{
    actuatorCallback = callback;
}


template<typename T>
void MoodyActuator<T>::begin()
{
    webServer.on("/api/conn", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        String mac = WiFi.macAddress();
        StaticJsonDocument<MAX_JSON_CONN_SIZE> jsonDoc;

        jsonDoc["type"] = "actuator";
        jsonDoc["service"] = serviceName;
        jsonDoc["mac"] = mac;
        serializeJson(jsonDoc, resp);

        request->send(200, "application/json", resp);
    });

    webServer.on("/api/data", HTTP_GET, [this](AsyncWebServerRequest *request) {
        String resp;
        StaticJsonDocument<JSON_SIZE> jsonDoc;
        jsonDoc["payload"] = state;
        serializeJson(jsonDoc, resp);
        request->send(200, "application/json", resp);
    });

    auto* handler = new AsyncCallbackJsonWebHandler("/api/data", [this](AsyncWebServerRequest *request, JsonVariant &json) {
        String resp;
        if(request->method() == HTTP_PUT)
        {
            JsonObject jsonObj = json.as<JsonObject>();
            if(jsonObj.containsKey("payload"))
            {
                T payload = jsonObj["payload"].as<T>();
                actuatorCallback(payload);
                state = payload;
            }
            serializeJson(jsonObj, resp);
            request->send(200, "application/json", resp);
            return;
        }
        request->send(501, "text/plain", "");
    });

    webServer.addHandler(handler);
}

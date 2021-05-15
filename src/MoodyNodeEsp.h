#include <ssdpAWS.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <ArxTypeTraits.h>
#include <EspWifiManager.h>
#include <ESPAsyncWebServer.h>


#define WEB_SERVER_PORT    80
#define MAX_PAYLOAD_LEN    8
#define MAX_JSON_CONN_SIZE 150
#define JSON_SIZE          64
#define PUT_JSON_SIZE      64
#define MAX_SERVICE_LEN    16
#define MAX_STRMSG_SIZE    8


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
        bool serverStarted;

    protected:
        ssdpAWS ssdp;
        const char* serviceName;
        AsyncWebServer webServer;
        
        MoodyBase(const char* serviceName);
        virtual void begin() = 0;
    
    public:
        void loop();
        virtual ~MoodyBase() {};
};


template<typename T>
class MoodySensor: public MoodyBase {
    private:
        SensorCallback<T> sensorCallback;

    public:
        MoodySensor(const char* serviceName);
        void setAcquireFunction(SensorCallback<T> callback);
        void begin() override;
        virtual ~MoodySensor() {}
};


template<typename T>
class MoodyActuator: public MoodyBase {
    private:
        T state;
        ActuatorCallback<T> actuatorCallback;

    public:
        MoodyActuator(const char* serviceName);
        void setActuateFunction(ActuatorCallback<T> callback);
        void begin() override;
        virtual ~MoodyActuator() {}
};


/*
    Base implementation
*/

MoodyBase::MoodyBase(const char* serviceName) : ssdp(nullptr), serverStarted(false), 
    serviceName(serviceName), webServer(WEB_SERVER_PORT) 
{
    ssdp = ssdpAWS(&webServer);
}


void MoodyBase::loop()
{
    WiFiManager.loop();
    if(WiFiManager.isConnected())
    {
        if(!serverStarted)
        {
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
    snprintf(chipId, sizeof(chipId), "%u", ESP.getChipId());
    ssdp.begin(serviceName, chipId, modelName, modelNumber, manufacturer, manufacturerUrl);

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
    snprintf(chipId, sizeof(chipId), "%u", ESP.getChipId());
    ssdp.begin(serviceName, chipId, modelName, modelNumber, manufacturer, manufacturerUrl);

    
    webServer.on("/api/conn", HTTP_GET, [](AsyncWebServerRequest *request) {
        String resp;
        String mac = WiFi.macAddress();
        StaticJsonDocument<MAX_JSON_CONN_SIZE> jsonDoc;

        jsonDoc["type"] = "actuator";
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

    AsyncCallbackJsonWebHandler* handler = new AsyncCallbackJsonWebHandler("/api/data", [this](AsyncWebServerRequest *request, JsonVariant &json) {
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

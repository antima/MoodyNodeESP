#include <EspWifiManager.h>

template<typename T>
using SensorCallback = T (*)();

template<typename T>
using ActuatorCallback = void (*)(T);


template<typename T>
class MoodySensor {
    private:
        SensorCallback<T> callback;

    public:
        MoodySensor();
        setAcquireFunction(SensorCallback callback);
        void loop();
        virtual ~MoodySensor() {}
};


template<typename T>
class MoodyActuator {
    private:
        SensorCallback<T> callback;

    public:
        MoodyActuator();
        setActuateFunction(SensorCallback callback);
        void loop();
        virtual ~MoodyActuator() {}
};
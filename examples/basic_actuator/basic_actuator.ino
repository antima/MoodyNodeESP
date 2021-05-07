#include <MoodyNodeEsp.h>

MoodyActuator<int> actuator("example");

void acquireFunction(int payload)
{
    Serial.println(payload);
}

void setup()
{
    Serial.begin(115200);
    actuator.setActuateFunction(acquireFunction);
    actuator.begin();
}

void loop()
{
    actuator.loop();
}
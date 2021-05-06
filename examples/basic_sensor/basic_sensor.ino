#include <MoodyNodeEsp.h>

MoodySensor<int> sensor("example");

int acquireFunction()
{
    static int i = 0;
    return i++;
}

void setup()
{
    sensor.setAcquireFunction(acquireFunction);
    sensor.begin();
}

void loop()
{
    sensor.loop();
}
# MoodyNodeEsp

Build simple and ready to use applications on ESP devices, focusing only on the application logic. This is a battery-included library, with connection management capabilities (using the EspWifiManager lib by [antima.it](#https://github.com/antima/EspWifiManager)).

This library is part of the Moody project.

## Contents
  - [About](#about) 
  - [Installation](#installation-arduino-ide)
  - [How To](#how-to)
  - [License](#license)


## About

You can build sensor and actuator applications by exploiting the MoodySensor and MoodyActuator classes.

These classes are generic in the type of data that you acquire (sensor) or receive to actuate(actuator), which must be an arithmetic type. You can use the examples to understand the basic concepts of the library.

The Wifi connection management capabilities included, automatically manage credentials by allowing the user to insert them via a web browser interface. They are then saved to the non -volatile memory available, in order to be retrieved later. This process uses the [EspWifiManager antima library](#https://github.com/antima/EspWifiManager#about).

## Installation (Arduino IDE)

This library depends on the following libraries (links included):

- [ESP Async WebServer](#https://github.com/me-no-dev/ESPAsyncWebServer/tree/master/examples)
- [EspWifiManager](#https://github.com/antima/EspWifiManager)
- [ArxTypeTraits](#https://github.com/hideakitai/ArxTypeTraits)

After installing these libraries, clone this repository into your Arduino libraries folder, or click download zip from the upper right corner of this github page if you want to have a release with the latest features.

If you want download the zip version, you can add it to your environment by clicking Sketch -> Include Library -> Add -> .ZIP Library... and selecting the zip file you just downloaded.

## How To

Basic sensor example:

```c++
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
```

Basic actuator example:
```c++
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
```

## License

This library is released under the LGPL 2.1 license. 

- The ESP Async WebServer library used by this library is released under the LGPL 2.1 license.
- The EspWifiManager library used by this library is released under the LGPL 2.1 license.
- The ArxTypeTraits library used by this library is released under the MIT license.

For more information regarding the licensing of the Arduino API, ESP8266 Arduino core, Arduino IDE, ecc, you can check out [this page](https://github.com/esp8266/Arduino#license-and-credits).
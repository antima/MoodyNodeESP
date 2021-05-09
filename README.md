# MoodyNodeEsp

Build simple and ready to use applications on ESP devices, focusing only on the application logic. This is a battery-included library, with connection management capabilities (using the EspWifiManager lib by [antima.it](https://github.com/antima/EspWifiManager)), and a simple REST HTTP interface.

This library is part of the Moody project.

## Contents
  - [About](#about) 
  - [Installation](#installation-arduino-ide)
  - [How To](#how-to)
    - [Interfacing with the devices](#interfacing-with-the-devices)
    - [Simple examples](#simple-examples)
  - [License](#license)


## About

You can build sensor and actuator applications by exploiting the MoodySensor and MoodyActuator classes.

These classes are generic in the type of data that you acquire (sensor) or receive to actuate(actuator), which must be an arithmetic type. You can use the examples to understand the basic concepts of the library.

The Wifi connection management capabilities included, automatically manage credentials by allowing the user to insert them via a web browser interface. They are then saved to the non -volatile memory available, in order to be retrieved later. This process uses the [EspWifiManager antima library](https://github.com/antima/EspWifiManager#about).

The device information is exposed through HTTP APIs. You can request a sensor reading, change an actuator state, read its state, request information about the device, all through HTTP endpoints. 

## Installation (Arduino IDE)

MoodyNodeEsp depends on the following libraries:

- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
  - [ESPAsyncTcp](https://github.com/me-no-dev/ESPAsyncTCP) (Dependency of ESPAsyncWebServer)
- [EspWifiManager](https://github.com/antima/EspWifiManager)
- [ArxTypeTraits](https://github.com/hideakitai/ArxTypeTraits)

To install these dependencies, you can clone the repositories in your Arduino/libraries directory:

```bash
cd /path/to/Arduino/libraries

git clone https://github.com/me-no-dev/ESPAsyncWebServer
git clone https://github.com/me-no-dev/ESPAsyncTCP
git clone https://github.com/antima/EspWifiManager
git clone https://github.com/hideakitai/ArxTypeTraits
```


You can also install them by downloading the zip archives of the linked projects by clicking on the links, then on the **Code** button and on the **Download ZIP** button; you can then add them to your Arduino IDE environment you can add it to your environment by clicking Sketch -> Include Library -> Add -> .ZIP Library... and selecting the zip file you just downloaded.

After installing everything, clone this repository into your Arduino libraries folder, 

If you want download the zip version, you can click download zip from the upper right corner of this github page to obtain a release containing the latest features. You can then add it to your environment by clicking Sketch -> Include Library -> Add -> .ZIP Library... and selecting the zip file you just downloaded.

## How To

### Interfacing with the devices

Devices using this library expose a number of HTTP APIs through which an external actor can interact with them.

A sensor node is reachable at the following endpoints:

- **/api/conn [GET]**: a get to this endpoint returns data regarding the node type, the sensor service and its mac address, in json format.
- **/api/data [GET]**: a get to this endpoint requests a sensor reading. The result of the operation is returned as a json message, in the **payload** field.

An actuator node is reachable at the following endpoints:

- **/api/conn [GET]**: a get to this endpoint returns data regarding the node type and its mac address, in json format.
- **/api/data [GET]**: a get to this endpoint returns the current state of the actuator. The result of the operation is returned as a json message, in the **payload** field.
- **/api/data [PUT]**: a put to this endpoint will result in a change of state inside the actuator, which in turn will actuate the received command. The new state is then returned as a json message, in the **payload** field.
- 

### Simple examples

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
## Air quality station!

This is fun project using PI-PCIO and air quality sensor (SDC40) as a tiny home air quality station with a display!

Example: 

|Normal | Night mode|
|----|---|
|<img src=https://github.com/amirhm/homedisplay/assets/2778581/a1f8d635-4c1b-40b3-8faf-a33afd83bae5 width=200/>|<img src=https://github.com/amirhm/homedisplay/assets/2778581/f0dfb161-b109-411b-b4d7-3ef63efb2526 width=200/>|

## Waht is it?

1. Connects to the wifi and updates the timing info from the ntp server.
2. Connect to SDC40 Sensor for CO2 via I2C, get Temp and Humidity and CO2 level
3. Display Information on 2-inch Color display (ST7789) via spi.
3. Date and time and Counter.

## Hardware, how to connect

1. RPI-PICO W (to connect to wifi and update the time)
2. SDC40 Air Quality sensor 
3. ST7789 Display 

### Pinout


|ST7789|RPI-PICO||SDC4x|RP-PICO|
|-|-|-|-|-|
|BLK|GP15||SDA|GP24|
|DC|GP20||SCL|GP25|
|RESET|GP21||VCC|3.3Vout|
|CSN|GP17||GND|GND|
|SDA|GP19|
|SCL|GP18|
|VCC|3.3Vout|
|GND|GND|


## Build

to build the binary:
```bash
cd pico-sdk
git submodule update --init
cd ..
```
To get the time data from ntp server a wifi connection is required. To define the wifi ssid and password, either define env variables `WIFI_SSID` and `WIFI_PASSWORD` or just define inline with cmake command:

```bash
WIFI_SSID="TEST" WIFI_PASSWORD="pass" cmake -DPICO_BOARD=pico_w
```

then make a build folder and build via cmake:

```bash
mkdir build
cd build
cmake -DPICO_BOARD=pico_w -DPICO_SDK_PATH=./pico/pico-sdk ..

```

now you can build either with `cmake --build .` or  `make all -j`

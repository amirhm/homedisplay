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

now you can build either 

``` bash
cmake --build .
```
or

```bash
make all -j
```
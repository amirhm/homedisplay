to build:
```bash
cd pico-sdk
git submodule update --init
cd ..

```

then make a build folder and build via cmake:

```bash
mkdir build
cd build
cmake -DPICO_BOARD=pico_w -DPICO_SDK_PATH=./pico/pico-sdk ..
make test -j4

```

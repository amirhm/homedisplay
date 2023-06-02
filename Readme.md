to build:

cd pico-sdk
git submodule update --init

cd ..

then make a build folder and build via cmake:

mkdir build
cd build
cmake -DPICO_BOARD=pico_w -DPICO_SDK_PATH=/home/amirhm/Documents/pico/pico-sdk ..
make test -j4

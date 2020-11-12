#!/bin/bash 

set -ex

echo "Installing dependancies"
sudo apt update
sudo apt upgrade
sudo apt install cmake git sqlite3 -y

mkdir build
cd build

echo "Installing bcm2835"
wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz
tar -xzvf bcm2835-1.68.tar.gz
cd bcm2835-1.68/
./configure
sudo make
sudo make install
cd ..

echo "Installing mqtt.c"
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.6

cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_BUILD_STATIC=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPAHO_ENABLE_TESTING=OFF
sudo env "PATH=$PATH" cmake --build build/ --target install
sudo ldconfig
cd ..

cd ..
sudo rm build -r

echo "Creating Build folder in GatewayRPI"

cd GatewayRPI
mkdir build

var=$1
if [[ -n "$var" && "$var" == "build" ]]; then
    cd build
    cmake ..
    make
    echo "navigate to GatewayRPI/build and run the program with 'sudo GatewayRPI/GatewayRPI'"
fi

exit 0
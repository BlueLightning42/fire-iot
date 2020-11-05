# setup
first update the package list
`sudo apt update`
Optional but might fix any potential errors in the future __to upgrade anything in the pi__
`sudo apt upgrade`
Install required programs and libraries
`sudo apt install cmake git sqlite3 -y`
Install the bcm2835 library from source
`wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz`
`tar -xzvf bcm2835-1.68.tar.gz`
`cd bcm2835-1.68/`
`./configure`
`sudo make`
`sudo make install`

navigate to the folder you plan to download/build the gateway (for example /Fire) and clone from github if you haven't already.
`git clone https://github.com/BlueLightning42/fire-iot.git --recurse-submodules`

then for the gateway create a build directory
`cd /gateway/GatewayRPI`
`mkdir build`
`cd build`
run cmake and build
`cmake ..`
`make`

## TODO WRITE all troubleshooting. and write steps for setting up the apache server/gateway form

## troubleshooting


if you forgot to put --recurse-submodules then head into to top directory of the project and put 
`git submodule update --init --recursive`
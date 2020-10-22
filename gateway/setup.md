# setup
first update the package list
`sudo apt update`
Install required programs and libraries
`sudo apt install cmake git sqlite3 -y`
navigate to download folder (for example /Fire) and clone from github if you haven't already.
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
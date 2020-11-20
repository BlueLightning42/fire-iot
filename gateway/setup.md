# setup

# Quick setup

To help save time theres a install.sh script in this folder.
*To run it please clone this repository into a folder on the pi then navigate to the gateway folder and run the install.sh script*
### Steps

If git is not installed
* `sudo apt install git`

If you want a custom folder name make a folder and clone this repository into it
* `mkdir Fire`
* `cd Fire`
* `git clone https://github.com/BlueLightning42/fire-iot.git --recurse-submodules .`

otherwise just clone it into the fire-iot folder with
* `git clone https://github.com/BlueLightning42/fire-iot.git --recurse-submodules`

navigate to the gateway folder
* `cd gateway`

and run the install script
* `./install.sh`

by default the script updates installs and builds however you can pass optional parameters to disable options like so
* `./install.sh -ni`
* `-n` disables apt update/ apt upgrade
* `-i` disables downloading and reinstalling bcm and mqtt c libraries
* `-b` disables building the project with cmake
* `-r` optionally runs the program after installation.

after installing it head down to the *"running the program"* section

# Manual
*If you don't want to run random scripts (understandable) you can follow all the steps the install script takes manually*

first update the package list
* `sudo apt update`

Optional but might fix any potential errors in the future __to upgrade anything in the pi__
* `sudo apt upgrade`

Install required programs and libraries
* `sudo apt install cmake git sqlite3 -y`

Install the bcm2835 library from source
* `wget http://www.airspayce.com/mikem/bcm2835/bcm2835-1.68.tar.gz`
* `tar -xzvf bcm2835-1.68.tar.gz`
* `cd bcm2835-1.68/`
* `./configure`
* `sudo make`
* `sudo make install`

Install the mqtt.c library from source
* `git clone https://github.com/eclipse/paho.mqtt.c.git`
* `cd paho.mqtt.c`
* `git checkout v1.3.6`
* `cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_BUILD_STATIC=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON -DPAHO_ENABLE_TESTING=OFF`
* `sudo env "PATH=$PATH" cmake --build build/ --target install`
* `sudo ldconfig`

You can either leave those files or delete them now that you are done.

navigate to the folder you plan to download/build the gateway (for example /Fire) and clone from github if you haven't already.
* `git clone https://github.com/BlueLightning42/fire-iot.git --recurse-submodules .`

then for the gateway create a build directory
* `cd /gateway/GatewayRPI`
* `mkdir build`
* `cd build`

run cmake and build
* `cmake ..`
* `make`

# Running the program
Make sure you are in the correct directory (GatewayRPI/build)
now all you have to do is run the program. - __I'm not sure yet if it can be run without root so for now__
* `sudo GatewayRPI/GatewayRPI`

## TODO WRITE all troubleshooting. and write steps for setting up the apache server/gateway form

## troubleshooting


if you forgot to put --recurse-submodules then head into to top directory of the project and put 
`git submodule update --init --recursive`
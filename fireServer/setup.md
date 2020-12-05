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
* `cd fireServer`

and run the install script
* `./install.sh`

by default the script updates installs and builds however you can pass optional parameters to disable options like so
* `./install.sh -ni`
* `-n` disables apt update/ apt upgrade
* `-i` disables downloading and reinstalling mqtt c libraries
* `-b` disables building the project with cmake
* `-a` disables setting up apache.
* `-r` optionally runs the program after installation.

after installing it head down to the *"running the program"* section

# Manual
*If you don't want to run random scripts (understandable) you can follow all the steps the install script takes manually*

You can read through each section of the install script and copy paste relevant parts- removing steps from here as more stuff keeps being added to the install script and its unmaintainable to make sure these match

# Running the program
Make sure you are in the correct directory (GatewayRPI/build)
now all you have to do is run the program. - __I'm not sure yet if it can be run without root so for now__
* `sudo alarm_monitoring/alarm_monitoring`


## troubleshooting


if you forgot to put --recurse-submodules then head into to top directory of the project and put
`git submodule update --init --recursive`

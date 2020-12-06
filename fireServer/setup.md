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
Make sure you are in the correct directory (alarm_monitoring/build)
now all you have to do is run the program.
* `alarm_monitoring/alarm_monitoring`


## troubleshooting


if you forgot to put --recurse-submodules then head into to top directory of the project and put
* `git submodule update --init --recursive`

If you get an error inserting into the stored_devices database with either the form or the monitoring app make sure the filepermissions are set for the folder/file in `/var/lib/fireiot/stored_devices.db`
*it should have already been set in the install script and running the program for the first time but make sure the database file and fireiot folder are both in the fire_iot group that both www-data and pi have been added to the fire_iot group and that both the folder and file have read/write permissions for the group settings.*
as long as the permissions for the folder are setup with `umask 002` `chgrp fire_iot .` `chmod g+s .` in the `/var/lib/fireiot/` folder then the program should correctly set its own permissions when first creating the file. you can test by deleting the database and having it remake itself one.

#include "headers/MonitorApp.h"

// Example program connecting to the gateway library with default options.

int main(){
	Monitor app;
	return 0;
}

/*
Relevant Header/source files

Gateway.h -Main program. 
- Controls data stored on the heap/ tracks devices
- Stores minimal information from a config file.
- Is in charge of the program mainloop.

logging.h
- Exposes the log( warning level, fmt, args) function
- Does not need to be touched or used but provides a nice interface to log stuff to both the console and a file.
- Potentially going provide an option to disable it on compile or control the level it logs. 

Storage.h
- provides structure of Device object 
- provides functions to load and store Devices from a sqlite3 databse.
- provides the makeAlert(id) function to load device info from database.

Communication.h
- has message info
- has a socket interface to recive packets in packet_communication
- and MQTT seperated into MQTT_communication.cpp.
- exposes the functions used to both recive messages and send them.

*/
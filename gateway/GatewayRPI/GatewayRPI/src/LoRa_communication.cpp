#include "../headers/Communication.h"
#include "../headers/Gateway.h"
/*
How this is setup is completely dependant upon how the LoRa library works. 

if readMessage() only works while theres a message and the message dissapears if not read then a seperate thread is needed

the thread would work like a interupt with some implementation of a concurent vector/vector that can be passed to the main thread

otherwise/my first attempt is going be every time this function is run Check for a message.
testing is needed if that message is lost if not read.

*/

void Gateway::pollMessages() {
	/* Add all LoRa messages recived to the messages vector
	 * TODO: ALL OF THIS
	 */

}
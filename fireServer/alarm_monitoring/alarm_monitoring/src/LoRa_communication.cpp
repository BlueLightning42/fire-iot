#include "../headers/Communication.h"
#include "../headers/Gateway.h"

#ifdef __linux__

/*
How this is setup is completely dependant upon how the LoRa library works. 

if readMessage() only works while theres a message and the message dissapears if not read then a seperate thread is needed

the thread would work like a interupt with some implementation of a concurent vector/vector that can be passed to the main thread

otherwise/my first attempt is going be every time this function is run Check for a message.
testing is needed if that message is lost if not read.

*/


// initial attempt https://www.airspayce.com/mikem/arduino/RadioHead/ if this doesn't run on the pi going use this fork https://github.com/hallard/RadioHead (much older commit so trying new official first)
// I need a driver and a manager
// RH_RF95 Works with Semtech SX1276/77/78/79, Modtronix inAir4 and inAir9, and HopeRF RFM95/96/97/98 and other similar LoRa capable radios. Supports Long Range (LoRa) with spread spectrum frequency hopping, large payloads etc. FSK/GFSK/OOK modes are not (yet) supported.
// https://www.airspayce.com/mikem/bcm2835/ needs to be downloaded and init...cant figure out if I can make a submodule yet

#include <bcm2835.h>
#include <unistd.h>
#include <RHutil/RasPi.h>
#include <RH_RF95.h> // I think I'm going have to use either this without the manager or figure out a different lib
//#include "./lib/RadioHead/RHDatagram.h"


// copy pasted defines... mod based on how the board is setup

// see https://github.com/hallard/RadioHead/blob/master/examples/raspi/rf95/rf95_server.cpp for setup

#ifndef OUTPUT
  #define OUTPUT BCM2835_GPIO_FSEL_OUTP
#endif

#ifndef INPUT
  #define INPUT BCM2835_GPIO_FSEL_INPT
#endif

#ifndef NOT_A_PIN
  #define NOT_A_PIN 0xFF
#endif

#define RF_CS_PIN  RPI_V2_GPIO_P1_24 // Slave Select on CE0 so P1 connector pin #24
#define RF_IRQ_PIN RPI_V2_GPIO_P1_22 // IRQ on GPIO25 so P1 connector pin #22
#define RF_RST_PIN RPI_V2_GPIO_P1_29 // RPI_V2_GPIO_P1_29 not defined...figure out what pin is the RST pin if used.


// Our RFM95 Configuration 
// The frequency range designated to LoRa Technology in the United States, Canada and South America is 902 to 928 MHz.
#define RF_FREQUENCY  915.00
#define RF_NODE_ID    1

using uint = unsigned int;


// Create an instance of a driver
RH_RF95 rf95(RF_CS_PIN, RF_IRQ_PIN);

bool LoRa_active = false;

void initLoRa(){
	if (!bcm2835_init()) { // if the lora device is plugged in correctly this should work
		log(logging::critical, "Error in bcm2835 init. LoRa not initialed! debug-(file:{} line:{})", __FILE__, __LINE__);
		return;
	}else{
		if( geteuid() != 0 ){
			// I think bcm only needs spi if not root.
			if(!bcm2835_spi_begin()){
				log(logging::critical, "Error begining bcm2835 spi. LoRa not initialized! debug-(file:{} line:{})", __FILE__, __LINE__);
				return;
			}
		}
		
		//this section hangs system
		// reading this https://www.airspayce.com/mikem/bcm2835/index.html "arious versions of Rasbian will crash or hang if certain GPIO pins are toggled"
		//   A workaround is to add this line to your /boot/config.txt:
		//   dtoverlay=gpio-no-irq
		//   but that doesn't work.
#ifdef RF_IRQ_PIN
		log(logging::info, "IRQ=GPIO{}", RF_IRQ_PIN);
		// IRQ Pin input/pull down
		pinMode(RF_IRQ_PIN, INPUT);
		bcm2835_gpio_set_pud(RF_IRQ_PIN, BCM2835_GPIO_PUD_DOWN);
		// Now we can enable Rising edge detection
		bcm2835_gpio_ren(RF_IRQ_PIN);
#endif

#ifdef RF_RST_PIN
		log(logging::info, "RST=GPIO{}", RF_RST_PIN);
		// Pulse a reset on module
		pinMode(RF_RST_PIN, OUTPUT);
		digitalWrite(RF_RST_PIN, LOW );
		bcm2835_delay((uint)150);
		digitalWrite(RF_RST_PIN, HIGH );
		bcm2835_delay((uint)100);
#endif
	}
	fmt::print("Got here 0\n");

	if (!rf95.init()) {
		log(logging::critical, "RF95 module init failed, Please verify wiring/module. debug-(file:{} line:{})", __FILE__, __LINE__);
		return;
	}else{
		LoRa_active = true;
		fmt::print("Got here 1\n");

		rf95.setTxPower(14, false);

		// You can optionally require this module to wait until Channel Activity
		// Detection shows no activity on the channel before transmitting by setting
		// the CAD timeout to non-zero:
		//rf95.setCADTimeout(10000);
		fmt::print("Got here 2\n");

		// Adjust Frequency
		rf95.setFrequency(RF_FREQUENCY);
		fmt::print("Got here 3\n");

		// If we need to send something
		rf95.setThisAddress(RF_NODE_ID);
		fmt::print("Got here 4\n");
		rf95.setHeaderFrom(RF_NODE_ID);
		fmt::print("Got here 5\n");

		// Be sure to grab all node packet 
		// we're sniffing to display, it's a demo
		rf95.setPromiscuous(true);
		fmt::print("Got here 6\n");

		// We're ready to listen for incoming message
		rf95.setModeRx();
		fmt::print("Got here 7\n");
	}
}

void closeLoRa(){
	bcm2835_close();
}

void Gateway::pollMessages() {
	/* Add all LoRa messages recived to the messages vector
	 * TODO: ALL OF THIS
	 */
	if(LoRa_active){

#ifdef RF_IRQ_PIN
	  // We have a IRQ pin ,pool it instead reading
	  // Modules IRQ registers from SPI in each loop

	  // Rising edge fired ?
	  if (bcm2835_gpio_eds(RF_IRQ_PIN)) {
		// Now clear the eds flag by setting it to 1
		bcm2835_gpio_set_eds(RF_IRQ_PIN);
		//printf("Packet Received, Rising event detect for pin GPIO%d\n", RF_IRQ_PIN);
#endif
		uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
		uint8_t len  = sizeof(buf);
		uint8_t from = rf95.headerFrom();
		uint8_t to   = rf95.headerTo();
		uint8_t id   = rf95.headerId();
		uint8_t flags= rf95.headerFlags();;
		int8_t rssi  = rf95.lastRssi();


		if (rf95.recv(buf, &len)) {
			// testing recived different from regular logging/will remove once it works.
			fmt::print(fg(fmt::color::medium_purple), "Packet[{}] #{} => #{}} {}dB: ", len, from, to, rssi);
			for (size_t i = 0; i< len; i++){
				fmt::print(fg(fmt::color::medium_purple), " {}", buf[i]);
			}
		} else {
			Serial.print("receive failed");
		}

#ifdef RF_IRQ_PIN
	  }
#endif
	}
}
// once I get LoRa active in the sense that it can read messages without freezing on GPIO move to here
void Gateway::makeMessageThread() {

}

#else
void initLoRa() {
	log(logging::warn, "Running on windows- LoRa functions are not setup");
}
void closeLoRa() {
	// pass
}
#include <iostream>

//Gateway::std::vector<Message> recived;

void Gateway::makeMessageThread() {
	message_thread = std::thread([&] {
		std::string s;
		bool error = false;
		while (!error && std::getline(std::cin, s, '\n')) {
			auto lock = std::unique_lock<std::mutex>(m);
			message_recived = true;
			if (s == "quit") {
				error = true;
			}
			auto type = typ::heartbeat;
			if (s.rfind("a", 0) == 0) {
				type = typ::alarm;
				s = s.substr(1);
			}
			try {
				uint16_t num = std::stoi(s);
				Message fake{0, num, type, 0 };
				recived.emplace_back(fake);
			}
			catch (...) {}
			lock.unlock();
		}
	});
}
void Gateway::pollMessages() {
	// fake message sending for testing.
	
	auto lock = std::unique_lock<std::mutex>(m);
	if (message_recived) {
		messages = std::move(recived);
		message_recived = false;
	}
	lock.unlock();

}
#endif
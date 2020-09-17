#include "GatewayRPI.h" //basically pch.h but not precompiled rn

#include "headers/Gateway.h"


int main(){
	//testing
	log(logging::info, "info example {}\n", "works okay!");
	log(logging::warn, "warn example {}\n", 24);
	log(logging::critical, "critical example {} {} {}\n", 12, 'a', "shutting down");

	Gateway app;
	return 0;
}

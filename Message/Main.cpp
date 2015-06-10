
#include <RMessage.h>
#include "RMessageArray.h"
#include <iostream>
#include <string>
#include <Reef.h>
#include <ReefSatellite.h>

int main(int argc, char *argv[]){

	int version = 0;
	 
	
	if (version == 4){
		/*Satellite2*/
		ReefSatellite sat;
		RMessage pubMessage1;

		sat.connect("Sat2", "127.0.0.1:5573"); //initiate version rep-port
		std::cout << "Sat connected to Reef" << std::endl;
		int i = 0;

		while (true){
			pubMessage1.addTag("Numbers2Crunch");
			pubMessage1.addTag("Sat1");
			pubMessage1.addSimplex("num1", 1);
			pubMessage1.addSimplex("num2", 2);
			pubMessage1.addSimplex("msgCount", ++i);

			sat.pub(pubMessage1);
			Sleep(700);
			pubMessage1 = RMessage();
		}
		/*Satellite2*/
	}

	else if (version == 3){
		/* Satellite1*/
		ReefSatellite sat;
		RMessage subMessage1;
		RMessage pubMessage1;

		sat.connect("Sat1", "127.0.0.1:5565"); //initiate version rep-port
		std::cout << "Sat connected to Reef" << std::endl;

		int tmp;
		bool pub = false;

		while (true){
			if (sat.receive(subMessage1)){
				if (subMessage1.containsAnyOf("Numbers2Crunch")){
					pub = true;
					std::cout << "Numbers to crunch: " << subMessage1.getInt("num1") << " + " << subMessage1.getInt("num2") << std::endl;
					std::cout << "Crunching ...." << std::endl;
					Sleep(4000);
					tmp = subMessage1.getInt("num1") + subMessage1.getInt("num2");
					pubMessage1.addTag("crunchedNumbers");
					pubMessage1.addSimplex("Numbers", tmp);
					sat.pub(pubMessage1);
				}
				else {
					std::cout << "FAILURE: Sat1 Received wrong message" << std::endl;
				}
				pubMessage1 = RMessage();
			}
			else{
				Sleep(4000);
			}


		}
		/* Satellite1*/
	}



	else if (version == 2){
		/* Server 3*/
		Reef testReef;
		RMessage subMessage1;
		RMessage pubMessage1;

		testReef.initiate("Start", "127.0.0.1", "5571", "5573");
		std::cout << "Reef initiated" << std::endl;

		testReef.connect("Server3", "127.0.0.1:5571", "127.0.0.1:5569");
		std::cout << "Reef connected" << std::endl;


		pubMessage1.addTag("server3tag");
		pubMessage1.addSimplex("server3key", "server3data");



		while (true){
			testReef.subMessage(subMessage1);
			testReef.pubMessage(pubMessage1);
			Sleep(700);
		}

		/*Server 3*/
	}
	else if (version == 1){
		/* Server 2*/
		Reef testReef;
		RMessage subMessage1;

		testReef.initiate("Start", "127.0.0.1", "5567", "5569");
		testReef.connect("Server2", "127.0.0.1:5567", "127.0.0.1:5565");
		std::cout << "Reef connected" << std::endl;

		testReef.addTag("crunchedNumbers");

		while (true){
			while (testReef.subMessage(subMessage1)){
				if (subMessage1.containsAnyOf("crunchedNumbers")){
					std::cout << "Received crunched Numbers: " << subMessage1.getInt("Numbers") << std::endl;
				}
				else {
					std::cout << "We should never get here" << std::endl;
				}
			}
		}

		/*Server 2*/
	}
	else if (version == 0){
		/* Server 1*/
		Reef testReef;
		RMessage subMessage;

		testReef.initiate("Server1", "127.0.0.1", "5563", "5565");
		std::cout << "Reef initiated" << std::endl;

		testReef.addTag("server3tag");

		while (true){
			if (testReef.subMessage(subMessage)){
				if (subMessage.containsAnyOf("server3tag")){
					std::cout << "Received Message from Server 3!" << std::endl;
				}
			}
		}

		/*Server 1*/
	}
	return 0;
}
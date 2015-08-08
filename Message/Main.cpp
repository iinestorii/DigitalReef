
#include <RMessage.h>
#include "RMessageArray.h"
#include <iostream>
#include <string>
#include <Reef.h>
#include <ReefSatellite.h>

int main(int argc, char *argv[]){
	
	int version = 0;
	/*
	std::string testString = "\"\"\"\"\"";
	std::cout << testString<< ", size: "<<testString.size()<< std::endl;

	CJsonObject objTest;
	objTest.AddPare("key", true);
	std::string testStringObject = objTest.ToString();
	std::cout << testStringObject << ", size: " << testStringObject.size() << std::endl;
	*/
	RMessage rmessage;

	std::string key0= "key";
	std::string str= "valdfgdgdgdgdg";

	bool boo = true;

	int inte = 999;

	RMessageBody inBody;
	inBody.addSimplex("key001", "valsdfgfgfgfgfgfgfgfgfgfgue");
	inBody.addSimplex("key002", false);
	inBody.addSimplex("key001", 999999999);

	RMessageArray inArray;
	inArray.addSimplex("value");
	inArray.addSimplex(false);
	inArray.addSimplex(9999);
	std::string tempStr;

	int answer = 0;
	rmessage.addTag("sat1");
	for (int i = 0; i <5; i++){
	
		if (i < 100){
			
			tempStr = key0 +  std::to_string(i);
			answer=rmessage.addSimplex(tempStr, str);
		}
		else if (i < 200){
			tempStr = key0 + std::to_string(i);
			answer = rmessage.addSimplex(tempStr, boo);
		}
		else if (i < 300){
			tempStr = key0 + std::to_string(i);
			answer = rmessage.addSimplex(tempStr, inte);
		}
		else if (i < 400){
			tempStr = key0 + std::to_string(i);
			answer = rmessage.addInnerBody(tempStr, inBody);
		}
		else{
			tempStr = key0 + std::to_string(i);
			answer = rmessage.addArray(tempStr, inArray);
		}
		if (answer == -1){
			std::cout << "body of message full!" << std::endl;
			answer = 0;
		}
		
	}

	std::cout << "getSize(): " << rmessage.getBody().size() << std::endl;

	if (version == 6){
		/* Server big Message test*/
		Reef testReef;

		testReef.initiate("Start", "127.0.0.1", "5567", "5569");
		testReef.connect("Server2", "127.0.0.1:5567", "127.0.0.1:5565");
		std::cout << "Reef connected" << std::endl;

		testReef.addTag("crunchedNumbers");
		
		while (true){
			testReef.pubMessage(rmessage);
			Sleep(100);
		}

		/*Server big Message test*/
	}

	else if (version == 5){ //big message test
		/* Satellite1*/
		ReefSatellite sat;
		RMessage subMessage1;

		sat.connect("sat1", "127.0.0.1:5565"); //initiate version rep-port
		std::cout << "Sat connected to Reef" << std::endl;

		while (true){
			if (sat.receive(subMessage1)){

				std::cout << "message received" << std::endl;
			}
			Sleep(4000);

		}
	}

	
	else if (version == 4){
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
			//subMessage.clear();
			
		}


		/*Server 1*/
	}
	return 0;
}


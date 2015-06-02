#include <RMessage.h>
#include "RMessageArray.h"
#include <iostream>
#include <string>
#include <Reef.h>
#include <ReefSatellite.h>

int main(int argc, char *argv[]){
	/*
	RMessage msg;
	msg.addSimplex("integer_1", 333);
	msg.addSimplex("integer_2", 7);
	msg.addSimplex("char_1", "test");
	msg.addSimplex("char_2", "test");
	msg.addSimplex("string_1", (std::string)"test");
	msg.addSimplex("string_2", (std::string)"test");
	msg.addSimplex("bool_1", false);
	msg.addSimplex("bool_2", true);

	RMessageBody body;
	body.addSimplex("integer_3", 5);
	body.addSimplex("integer_4", 7);
	body.addSimplex("char_3", "test");
	body.addSimplex("char_4", "test");
	body.addSimplex("string_3", (std::string)"test");
	body.addSimplex("string_4", (std::string)"test");
	body.addSimplex("bool_3", false);
	body.addSimplex("bool_4", true);

	RMessageArray array;
	array.addSimplex(5);
	array.addSimplex(7);
	array.addSimplex("test");
	array.addSimplex("test");
	array.addSimplex((std::string)"test");
	array.addSimplex((std::string)"test");
	array.addSimplex(false);
	array.addSimplex(true);

	//msg.addArray("array", array);

	RMessageArray array2;
	array2.addSimplex(5);
	array2.addSimplex(7);
	array2.addSimplex("test");
	array2.addSimplex("test");
	array2.addSimplex((std::string)"test");
	array2.addSimplex((std::string)"test");
	array2.addSimplex(false);
	array2.addSimplex(true);

	RMessageBody body2;
	body2.addSimplex("integer_5", 5);
	body2.addSimplex("integer_6", 7);

	body.addArray("array_in_body", array2);
	body.addInnerBody("body2", body2);

	RMessageArray array3;
	array3.addSimplex(5);
	array3.addInnerBody(body);

	msg.addArray("array3", array3);
	msg.addInnerBody("innerbody", body);
	std::string builder = msg.getBody();
	RMessage msg2;
	msg2.initiateWithJson(builder);
	msg2.addSimplex("teststring", "test_string");
	msg2.addSimplex("testinteger", 13);
	msg2.addSimplex("testfalse", false);
	msg2.addSimplex("testtrue", true);

	RMessageArray array4;
	array4.addSimplex("string in array4");
	array4.addSimplex(4);
	array4.addSimplex(true);
	RMessageBody body3;
	body3.addSimplex("body3-Int",3);
	array4.addInnerBody(body3);
	msg2.addArray("array4 in msg2", array4);

	RMessageBody body4;
	body4.addSimplex("Body4-String", "String in body4");
	body4.addSimplex("Body4-Int", 5);
	body4.addSimplex("Body4-Bool", false);
	msg2.addInnerBody("body4", body4);

	std::cout << "msg2 TestString:: " << msg2.getString("teststring") << std::endl;
	std::cout << "msg2 TestInt:: " << msg2.getInt("testinteger") << std::endl;
	bool testfalse = msg2.getBool("testfalse");
	std::cout << "msg2 TestBool false:: " << testfalse << std::endl;
	std::cout << "msg2 TestBool true:: " << msg2.getBool("testtrue") << std::endl;

	RMessageArray array4Extract = msg2.getArray("array4 in msg2");
	std::cout << "msg2 TestArray4:: "  << std::endl;
	std::cout <<  array4Extract.toString() << std::endl;
	std::cout << array4Extract.getString(0) << std::endl;
	std::cout << array4Extract.getInnerBody(3).toString() << std::endl;

	RMessageBody body4Extract = msg2.getInnerBody("body4");
	std::cout << "msg2 Body4 extracted:: " << std::endl;
	std::cout << body4Extract.toString() << std::endl;
	
	msg2.addTag("testtag");
	msg2.addTag((std::string)"testtag2");
	msg2.addTag("testtag3");
	msg2.addTag("testtag4");
	std::vector<std::string> string_vector_true;
	std::vector<std::string> string_vector_false;
	string_vector_true.push_back("others1");
	string_vector_true.push_back("testtag3");
	string_vector_true.push_back("others2");
	string_vector_false.push_back("others1");
	string_vector_false.push_back("testtagfalse");
	string_vector_false.push_back("others2");
	std::cout << "False_Tags: " << msg2.containsAnyOf(string_vector_false) << std::endl;
	std::cout << "True_Tags: " << msg2.containsAnyOf(string_vector_true) << std::endl;
	
	*/
	int version =0;

		if (version == 4){
			/* Server 4*/
			Reef testReef;
			RMessage subMessage1;
			RMessage pubMessage1;


			testReef.initiate("Start", "127.0.0.1", "5575", "5577");
			std::cout << "Reef initiated" << std::endl;

			testReef.connect("Server4", "127.0.0.1:5575", "127.0.0.1:5565"); //connects to Server 1
			std::cout << "Reef connected" << std::endl;

			testReef.addTag("sat1tag");

			pubMessage1.addTag("server4tag");
			pubMessage1.addTag("Sat1");
			pubMessage1.addSimplex("server4key", "server4data");


			while (true){
				testReef.pubMessage(pubMessage1);
				while (testReef.subMessage(subMessage1)){
					if (subMessage1.containsAnyOf("sat1tag")){
						std::cout << "Server 4 received Message from sat1" << std::endl;
						
					}
					else {
						std::cout << "FAILURE: Server 4 Received wrong message" << std::endl;
					}
				}			
				Sleep(1000);
			}

			/*Server 4*/
		}
	else if (version == 3){
		/* Satellite Version 1*/
		ReefSatellite sat;
		RMessage subMessage1;
		RMessage pubMessage1;
		RMessage pubMessage2;


		sat.connect("Sat1", "127.0.0.1:5565"); //initiate version rep-port
		std::cout << "Sat connected to Reef" << std::endl;


		pubMessage1.addTag("sat1tag");
		pubMessage1.addSimplex("sat1key", "pub");

		pubMessage2.addTag("sat1tag");
		pubMessage2.addSimplex("sat1key", "pub&receive");

		while (true){
			sat.pub(pubMessage1);
			Sleep(2000);
			if (sat.pubAndReceive(pubMessage2, subMessage1)){
				std::cout << "nach sat.pubAndReceive(...)" << std::endl;
				if (subMessage1.containsAnyOf("server4tag")){
					std::cout << "Message received from Server 4 via pubAndReceive(...)";
				}				else {
					std::cout << "FAILURE: Sat1 Received wrong message" << std::endl;
				}
			}
			Sleep(2000);
			while (sat.getWaitingMsgs()){
				std::cout << "inside while( ... )" << std::endl;
				if (sat.receive(subMessage1)){
					if (subMessage1.containsAnyOf("server4tag")){
						std::cout << "Message received from Server 4 via receive(...)";
					}
					else {
						std::cout << "FAILURE: Sat1 Received wrong message" << std::endl;
					}
				}
			}
			Sleep(2000);
		}

		/*Connect Version 1*/
	}
	
	else if (version == 2){
		/* Connect Version 2*/
		Reef testReef;
		RMessage subMessage1;
		RMessage pubMessage1;
		RMessage pubMessage2;

		testReef.initiate("Start", "127.0.0.1", "5571", "5573");
		std::cout << "Reef initiated" << std::endl;

		testReef.connect("Dritte", "127.0.0.1:5571", "127.0.0.1:5569");
		std::cout << "Reef connected" << std::endl;

		testReef.addTag("Ping");
		testReef.addTag("echo");



		pubMessage1.addTag("coral2");
		pubMessage1.addSimplex("coral2key", "coral2data");



		while (true){
			while (testReef.subMessage(subMessage1)){

				if (subMessage1.containsAnyOf("Ping")){
				std::cout << "Ping" << std::endl;
				}	else if (subMessage1.containsAnyOf("echo")){
				std::cout << "Pong" << std::endl;
				}
			}
			testReef.pubMessage(pubMessage1);
			Sleep(700);
		}

		/*Connect Version 2*/
	}
	else if (version == 1){
		/* Connect Version 1*/
		Reef testReef;
		RMessage subMessage1;
		RMessage pubMessage1;
		RMessage pubMessage2;

		testReef.initiate("Start", "127.0.0.1", "5567", "5569");
		std::cout << "Reef initiated" << std::endl;

		testReef.connect("Zweite", "127.0.0.1:5567", "127.0.0.1:5565");
		std::cout << "Reef connected" << std::endl;

		testReef.addTag("Ping");
		testReef.addTag("coral0");
		testReef.addTag("coral2");

		pubMessage1.addTag("echo");
		pubMessage1.addSimplex("echo", "Pong");


		pubMessage2.addTag("coral1");
		pubMessage2.addSimplex("coral1key", "coral1data");

		bool ping=false;

		while (true){
		testReef.pubMessage(pubMessage2);
			while(testReef.subMessage(subMessage1)){
				if (subMessage1.containsAnyOf("Ping")){
					std::cout << "Ping" << std::endl;
					ping = true;					
				}
				else if (subMessage1.containsAnyOf("coral0")){
					std::cout << subMessage1.getString("coral0key") << std::endl;
				}
				else if (subMessage1.containsAnyOf("coral2")){
					std::cout << subMessage1.getString("coral2key") << std::endl;
				}
			}
			if (ping){
				testReef.pubMessage(pubMessage1);
				ping = false;
			}
			Sleep(700);
		}

		/*Connect Version 1*/
	}
	else if (version == 0){
		/* Initiate Version*/
		Reef testReef;
		RMessage subMessage;
		RMessage pubMessage1;
		RMessage pubMessage2;

		testReef.initiate("Start", "127.0.0.1", "5563", "5565");
		std::cout << "Reef initiated" << std::endl;

		testReef.addTag("echo");
		testReef.addTag("coral1");
		testReef.addTag("coral2");

		pubMessage1.addTag("Ping");
		pubMessage1.addSimplex("echo", "Pong");


		pubMessage2.addTag("coral0");
		pubMessage2.addSimplex("coral0key", "coral0data");
		bool pong_received = false;
		bool pong = false;

		while (true){
			if (!pong_received){
				testReef.pubMessage(pubMessage1);
			}
			Sleep(700);
			while (testReef.subMessage(subMessage)){
				if (subMessage.containsAnyOf("echo")){
					pong_received = true;
					pong = true;					
					std::cout << "Pong" << std::endl;
					
				}
				else if (subMessage.containsAnyOf("coral1")){
					std::cout << subMessage.getString("coral1key") << std::endl;
				}
				else if (subMessage.containsAnyOf("coral2")){
					std::cout << subMessage.getString("coral2key") << std::endl;
				}
			}
			if (pong){
				testReef.pubMessage(pubMessage1);
				pong = false;
			}
			testReef.pubMessage(pubMessage2);
		}

		/*Initiate Version*/
	}
	return 0;
}
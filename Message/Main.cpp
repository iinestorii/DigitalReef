
#include <RMessage.h>
#include "RMessageArray.h"
#include <iostream>
#include <string>
#include <ReefServer.h>
#include <ReefSatellite.h>

int main(int argc, char *argv[]){
	////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
	There are different use cases in this Main.cpp to highlight some of the capabilities of the Digital-Reef
	Depending on the value of int switcher this will compile 1 of 7 different ReefServer/Satellites, these are
	examples and their switcher values:

	PingPong-Example
	switcher = 0	PingPong-Server
	switcher = 1	Pinger-Server, initializes the Reef

	MaxMessageLength-Example:
	switcher = 2	Builds and tests Message with max length

	Worker-Example
	switcher = 3	server1, initializes the Reef
	switcher = 4	satellit2
	switcher = 5	server2
	switcher = 6	satellit1

	More specific information can be found in the comments at the examples
	or in the appendix of the corresponding paper to this project
	
	*/
	////////////////////////////////////////////////////////////////////////////////////////////////////////////


	int switcher = 1; //switches witch example will be compiled

	/*
	PingPong-Example:
	PingPong-Server,answers to messages ping with pong, and messages pong with ping
	*/
	if (switcher == 0){	
		ReefServer ppServer;

		//initiate the Server with his name, ip, port of publish- and port of reply-socket
		ppServer.initiate("PingPong", "127.0.0.1", "5567", "5569");

		//connect to pinger-server at adress "127.0.0.1:5565" with alias PingPong and own publisher-adress
		ppServer.connect("PingPong", "127.0.0.1:5567", "127.0.0.1:5565");
		std::cout << "Reef connected" << std::endl;
		//ppServer is now a full member of the Reef

		//Add tags that interest him, Messages with these tags will be received
		ppServer.addTag("Ping");
		ppServer.addTag("Pong");

		//build 2 Messages as replies to incoming pings or pongs
		RMessage pongAntwort;
		RMessage pingAntwort;
		pongAntwort.addTag("pongAntwort");
		pongAntwort.addSimplex("Antwort", "Ping!");
		pingAntwort.addTag("pingAntwort");
		pingAntwort.addSimplex("Antwort", "Pong!");

		//empty message to receive an interesting message
		RMessage message;

		//main loop
		while (true){
			if (ppServer.subMessage(message)){ //true = RMessage received and stored in message
				if (message.containsAnyOf("Ping")){ //if it's a ping Message
					ppServer.pubMessage(pingAntwort); //publish a pong
				}
				else if (message.containsAnyOf("Pong")){ //if it's a pong Message
					ppServer.pubMessage(pongAntwort); //publish a ping
				}
				else{
					//ReefServer::subMessage only returns with Messages to corosponding Tags in the Tag-List
					std::cout << "We should never get here" << std::endl;
				}
			}
		}
	}

	/*
	PingPong-Example:
	Pinger-Server,sends pings and pongs and prints answers
	*/
	else if (switcher == 1){		 /* Pinger*/
		ReefServer pinger;
		//initiate the Reef
		pinger.initiate("Pinger", "127.0.0.1", "5563", "5565");
		std::cout << "Reef initiated" << std::endl;

		//build the ping and pong messages
		RMessage ping;
		RMessage pong;
		ping.addTag("Ping");
		pong.addTag("Pong");
		ping.addSimplex("key1", "body1");
		pong.addSimplex("key1", "body1");		

		//Tags of Messages that interest pinger
		pinger.addTag("pingAntwort");
		pinger.addTag("pongAntwort");
		
		//empty Message to story interesting Messages
		RMessage subMessage;
		while (true){
			//as Long as there are Messages waiting at the Subscribe-Socket
			while(pinger.subMessage(subMessage)){
					std::cout << "received :" << subMessage.getString("Antwort") << std::endl;
			}
			//send the pings and pongs
			pinger.pubMessage(ping);
			pinger.pubMessage(pong);
			Sleep(200);
		}
	}

	/*
	MaxMessageLength-Example:
	Builds a Message until it's full, tests if it's size in in allowed range
	*/
	else if (switcher == 2){ //Max length Message test
		RMessage rmessage;				//Message we want to fill
		rmessage.addTag("test");

		std::string key0 = "key";

		//Stuff we want to fill rmessage with:
		std::string str = "valdfgdgdgdgdg";  //some string
		bool boo = true;					//some bool
		int inte = 999;						//some int
		RMessageBody inBody;				//some inner RMessageBody
		inBody.addSimplex("key001", "valsdfgfgfgfgfgfgfgfgfgfgue");
		inBody.addSimplex("key002", false);
		inBody.addSimplex("key001", 999999999); //test same key-detection
		RMessageArray inArray;			// some RMessageArray
		inArray.addSimplex("value");
		inArray.addSimplex(false);
		inArray.addSimplex(9999);


		std::string tempStr; //String we use to build new keys for eacht iteration of the loop
		int answer = 0;		 //if answer==-1 then rmessage full and we cant add any more
		
		for (int i = 0; i <500; i++){	//main-loop

			if (i < 100){	//the first 100 iterations add a string
				tempStr = key0 + std::to_string(i); //build a new key
				answer = rmessage.addSimplex(tempStr, str); //add the string, store returnValue in answer
			}
			else if (i < 200){ //same as i<100 but with a bool
				tempStr = key0 + std::to_string(i);
				answer = rmessage.addSimplex(tempStr, boo);
			}
			else if(i<300){ //same as i<100 but with an int
				tempStr = key0 + std::to_string(i);
				answer = rmessage.addSimplex(tempStr, inte);
			}
			else if (i < 400){ //same as i<100 but with an innerBody
				tempStr = key0 + std::to_string(i);
				answer = rmessage.addInnerBody(tempStr, inBody);
			}
			else { //same as i<100 but with an Array
				tempStr = key0 + std::to_string(i);
				answer = rmessage.addArray(tempStr, inArray);
			}

			if (answer == -1){ //the Message is full and forbids further adding of key/values
				std::cout << "body of message full!" << std::endl;
				break;
			}

		}
		std::cout << "Should be below 4920, getSize(): " << rmessage.getBody().size() << std::endl;
		Sleep(10000);
	}

	/*	
		Worker-Example: 
		Server 1, just sits there and lets Satellites connect to him, handles there Messages 
	*/
	else if (switcher == 3){		 
		ReefServer server1;
		server1.initiate("server1", "127.0.0.1", "5563", "5565");
		std::cout << "Reef initiated" << std::endl;

		RMessage subMessage;
		while (true){
			server1.subMessage(subMessage);
		}
	}

	/*
	Worker-Example:
	Satellite 2, connects to Server1 and sends a Message with Numbers to crunch every 700ms
	which will be reveived and worked on by the workers
	*/
	else if (switcher == 4){
		ReefSatellite sat2;
		//connect the satellite to server1
		sat2.connect("Sat2", "127.0.0.1:5565");
		std::cout << "Sat connected to Reef" << std::endl;

		//build the workOrder
		RMessage pubMessage1;
		pubMessage1.addTag("Numbers2Crunch");
		pubMessage1.addTag("Sat1"); //This will trigger Server 1 to safe the Message in the Queue for Sat1
		pubMessage1.addSimplex("num1", 1);
		pubMessage1.addSimplex("num2", 2);		
		int i = 0;
	
		while (true){
			//add value of workOrder-counter i to message
			pubMessage1.addSimplex("msgCount", ++i); //overwrites the old value
			std::cout << "WorkOrder sent!" << std::endl;
			//publish the workorder
			sat2.pub(pubMessage1);
			Sleep(1000);
		}
	}
	/*
	Worker-Example:
	Server 2, connects to Server1 and waits for the results of the workers
	*/
	if (switcher == 5){				
		ReefServer printer;
		printer.initiate("printer", "127.0.0.1", "5567", "5569");
		printer.connect("printer", "127.0.0.1:5567", "127.0.0.1:5565");
		printer.addTag("Result");
		RMessage message;
		while (true){
			if (printer.subMessage(message)){
				if (message.containsAnyOf("Result")){
					std::cout << "Result " << message.getInt("number") << " received: " << message.getInt("result")<< std::endl;
				}				
				else{
					std::cout << "We should never get here!" << std::endl; //subMessage should only return Messages with "Result" as tag
				}
			}
		}
	}

	/*
	Worker-Example:
	Satellite 1, used as Worker, connects to Server1 and waits for the workorders sent by satellite2
	Crunches the numbers and sends out the result, which will be documented by Server2
	5 Workers should be enough to handle the incoming workOrders
	*/
	else if (switcher == 6){
		ReefSatellite sat1;
		sat1.connect("Sat1", "127.0.0.1:5565");
		std::cout << "Sat connected to Reef" << std::endl;

		//result of workOrder, will be printed by Server2
		RMessage pubMessage1;		
		pubMessage1.addTag("Result");

		RMessage workOrder;
		while (true){
			if (sat1.receive(workOrder)){
				int i = workOrder.getInt("num1"); //get the numbers
				int j = workOrder.getInt("num2");
				std::cout << "Crunching some Numbers...." << std::endl;
				Sleep(5000); //crunching some highly complex numbers...
				int result = j + i;
				pubMessage1.addSimplex("number", workOrder.getInt("msgCount")); //overwrites the old value, forwards the OrderCount
				pubMessage1.addSimplex("result",result);
				sat1.pub(pubMessage1); //sends pubReques to Server1 who handles the publishing
				std::cout << "Result sent!" << std::endl;
			}
		}
	}

	return 0;
}


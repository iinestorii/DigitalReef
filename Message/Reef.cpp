#include "Reef.h"
#include <stdio.h>


Reef::Reef() :
	context(1),
	publisher(context, ZMQ_PUB),
	subscriber(context, ZMQ_SUB),
	req(context, ZMQ_REQ),
	rep(context, ZMQ_REP){
}


Reef::~Reef()
{
}
//returns 1 if everything worked as expected, other error codes will follow
int Reef::connect(std::string aka, std::string ownIp, std::string reefIp){
	//make a Request to connect to the reef, saves Reef Addresslist in adr_list
	connectRequest(aka, ownIp, reefIp);

	//use the adr_list to connect your sub to all pubs in the reef
	connectSubscribe();

	return 1;
}

void Reef::connectRequest(std::string aka, std::string ownIp, std::string reefIp){
	std::string str_constructor = "tcp://" + reefIp + ":5565";

	const char* ip_char = str_constructor.c_str();

	req.connect(ip_char);

	//determine length of message for the char buffer
	int msg_length = aka.length() + ownIp.length() + 3;

	//initialize message with specific length
	zmq::message_t message(msg_length);

	//saves the aka and ownIp in the data part of the message
	_snprintf((char *)message.data(), msg_length,
		"%s %s", aka, ownIp);

	//sends Message to Reef
	req.send(message);

	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);

	//get the json string from the reply
	std::istringstream iss(static_cast<char*>(reply.data()));
	std::string json;
	iss >> json;

	//parse the json to a JsonObject and save it as the new adr_list
	adr_list = *CJsonParser::Execute((jstring)json);
}

//subscribes to each member of the address list adr_list
void Reef::connectSubscribe(){
	//variables for the for-loop
	const CJsonValue* cvn;
	std::string address;
	std::string str_constructor;

	//iterator for the address-list
	std::vector <jstring> keys;
	adr_list.GetNames(keys);
	std::vector<jstring>::iterator it;

	//iterate over the address list and subscribe to each publisher
	for (it = keys.begin(); it != keys.end(); it++)
		{
			cvn = adr_list[(*it)];			
			address = cvn->ToString();
			std::string str_constructor = address + ":5565";
			subscriber.connect(str_constructor.c_str());
		}
	
}

//returns 1 if everything worked as expected, other error codes will follow
int Reef::initiate(std::string aka, std::string ip){
	publisher.bind("tcp://*:5563");
	rep.bind("tcp://*:5565");
	adr_list.AddPare(aka, ip);
	return 1;
}

void Reef::addTag(std::string tag){

}

void Reef::removeTag(std::string tag){

}

void Reef::pubMessage(RMessage msg){

}

RMessage Reef::receiveMessage(){
	zmq::message_t request;
	//  Wait for new request from client
	rep.recv(&request);

	std::istringstream iss(static_cast<char*>(request.data()));
	std::string aka;
	std::string ip;

	iss >> aka >> ip;
	adr_list.AddPare(aka, ip);
	//publish new Reef member, Subscriber add them to their adr_list and sub to it's publisher

	//reply with adr_list
	std::string adr_list_str = adr_list.ToString();

	//determine length of message for the char buffer
	int msg_length =adr_list_str.length()+2;

	//initialize message with specific length
	zmq::message_t reply(msg_length);

	memcpy((void *)reply.data(), adr_list_str.c_str(), msg_length);

	//sends Message to Reef
	req.send(reply);
}
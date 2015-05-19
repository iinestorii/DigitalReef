#include "Reef.h"
#include <stdio.h>



Reef::Reef() :
	context(1),
	publisher(context, ZMQ_PUB),
	subscriber(context, ZMQ_SUB),
	req(context, ZMQ_REQ),
	rep(context, ZMQ_REP) 
	/*items({{
		{ req, 0, ZMQ_POLLIN, 0 },
		{ subscriber, 0, ZMQ_POLLIN, 0 }
	}})	*/	
{}


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
	std::string str_constructor = "tcp://" + reefIp;

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
			subscriber.connect(address.c_str());
		}
	
}

//returns 1 if everything worked as expected, other error codes will follow
int Reef::initiate(std::string aka, std::string ip, std::string pubPort, std:: string repPort){
	std::string pubStr = "tcp://*:"+ pubPort;
	std::string repStr = "tcp://*:" + repPort;
	std::string adressListStr = ip + ":" + pubPort;
	publisher.bind(pubStr.c_str());
	rep.bind(repStr.c_str());
	adr_list.AddPare(aka, adressListStr);
	return 1;
}

//returns 1 if everything worked as expected, other error codes will follow
int Reef::initiate(std::string aka, std::string ip){
	std::string adressListStr = ip + ":5563";
	publisher.bind("tcp://*:5563");
	rep.bind("tcp://*:5565");
	adr_list.AddPare(aka, adressListStr);
	return 1;
}

//returns true if tag_list contains tag
bool Reef::findTag(std::string tag){
	std::vector<std::string>::iterator findIter = std::find(tag_list.begin(), tag_list.end(), tag);
	if (findIter != tag_list.end()){
		return true;
	}
	else{
		return false;
	}
}

//adds a tag to the tag_list if it isn't already in there
//tag_list is used to filter incoming subscription messages for data of interest
void Reef::addTag(std::string tag){
	if (!findTag(tag)){
		tag_list.push_back(tag);
	}	
}

//removes a tag of the tag_list
void Reef::removeTag(std::string tag){
	tag_list.erase(std::remove(tag_list.begin(), tag_list.end(), tag), tag_list.end());
}

void Reef::pubMessage(RMessage& msg){
	s_sendmore(publisher, "");
	s_send(publisher, msg.getBody());
}

//checks via tag_list if Message of interest has arrived
//returns pointer to interesting message
//returns null-pointer if no Message of interest was found
RMessage* Reef::subMessage(){
	//initialize items[] if it isn't already
	if (!itemsSet){
		itemsInit();
	}

	//Message that will be received
	zmq::message_t message;

	//test if there are incoming messages on subscriber or req socket
	zmq::poll(items, 2); 

	//resolve message at request-socket
	if (items[0].revents & ZMQ_POLLIN) 
	{
		/*TODO
		1. Convert to RMessage
		2. If Tag identifies as new Reefmember request, initiate admission
		3. If Tag identifies as Pub-Request of Satellite-Coral, forward the Message, reply with stored incoming messages for Satellite Coral
		4. If Tag identifies as personal request to be handelt in upper level logic, return message
		*/			req.recv(&message);
		std::cout << "Processing message from receiver" << std::endl;
		items[0].revents = 0;
	}
	//resolve message at subscribe-socket
	if (items[1].revents & ZMQ_POLLIN) 
	{
		/*TODO
		1. Convert to RMessage
		2. If Tag identifies as Message of interest for this Coral return it.
		3. If Tag identifies as Message of interest for Satelite Coral store it and reply it with next request of Satellite
		4. else ignore 
		*/
		subscriber.recv(&message);
		std::cout << "Processing message from subscriber" << std::endl;
		items[1].revents = 0;
	}
	return 0;
}

/*
*	C++, especially in vb13 has some Problems with initializing arrays in the constructor
*	we don't want to initialize items[] everytime we poll, so the initialization got outsorced to 
*	here and the bool itemsSet is used to check befor each Poll if items[] is initialized
*/
void Reef::itemsInit(){
	items[0] = { req, 0, ZMQ_POLLIN, 0 };
	items[1] = { subscriber, 0, ZMQ_POLLIN, 0 };
	itemsSet = true;
}

void Reef::receiveMessage(){

	
	zmq::message_t request;
	//  Get new request from client, don't block if no Message is in queue
	rep.recv(&request, ZMQ_NOBLOCK);

	// cheack if Message is received, if yes, proceed to add new Reef-Member
	if (request.size() != 0){
		std::istringstream iss(static_cast<char*>(request.data()));
		std::string aka;
		std::string ip;

		iss >> aka >> ip;
		adr_list.AddPare(aka, ip);

		//build RMessage with aka and ip to inform Reef about the new Member
		RMessage pubMember;
		pubMember.addSimplex("aka", aka);
		pubMember.addSimplex("ip", ip);
		pubMember.addTag("SYS_newMember");

		//publish the pubMember Message in the Reef
		pubMessage(pubMember);

		//reply to new Coral with adr_list
		std::string adr_list_str = adr_list.ToString();

		//determine length of message for the char buffer
		int msg_length = adr_list_str.length() + 2;

		//initialize message with specific length
		zmq::message_t reply(msg_length);

		memcpy((void *)reply.data(), adr_list_str.c_str(), msg_length);

		//sends Message to new Coral
		req.send(reply);
	}
}
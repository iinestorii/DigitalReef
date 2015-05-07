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

void Reef::pubMessage(RMessage msg){
	s_sendmore(publisher, "");
	s_send(publisher, msg.getBody());
}

//checks via tag_list if Message of interest has arrived
//returns pointer to interesting message
//returns null-pointer if no Message of interest was found
RMessage* Reef::subMessage(){
	//  Read envelope with address
	std::string address = s_recv(subscriber);
	//  Read message contents
	std::string contents = s_recv(subscriber);
	
	RMessage msg;
	msg.initiateWithJson(contents);
	if (msg.containsAnyOf(tag_list)){
		return &msg;
	}
	else{
		return nullptr;
	}
}

void Reef::receiveMessage(){
	zmq::message_t request;
	//  Wait for new request from client
	rep.recv(&request);

	std::istringstream iss(static_cast<char*>(request.data()));
	std::string aka;
	std::string ip;

	iss >> aka >> ip;
	adr_list.AddPare(aka, ip);
	
	//build RMessage with aka and ip to inform Reef about the new Member
	RMessage pubMember;
	pubMember.addSimplex("aka",aka);
	pubMember.addSimplex("ip", ip);
	pubMember.addTag("SYS_newMember");

	//publish the pubMember Message in the Reef
	pubMessage(pubMember);

	//reply to new Coral with adr_list
	std::string adr_list_str = adr_list.ToString();

	//determine length of message for the char buffer
	int msg_length =adr_list_str.length()+2;

	//initialize message with specific length
	zmq::message_t reply(msg_length);

	memcpy((void *)reply.data(), adr_list_str.c_str(), msg_length);

	//sends Message to new Coral
	req.send(reply);
}
#include "ReefSatellite.h"
#include <stdio.h>



ReefSatellite::ReefSatellite() :
context(1),
req(context, ZMQ_REQ)
{}


ReefSatellite::~ReefSatellite()
{
}

//returns 1 if everything worked as expected
int ReefSatellite::connect(std::string aka, std::string reefIp){
	identity = aka;
	connectRequest(aka, reefIp);
	return 1;
}

void ReefSatellite::connectRequest(std::string aka, std::string reefIp){
	std::string ip_str = "tcp://" + reefIp;
	std::string mode = "1";
	req.connect(ip_str.c_str());	//connect to 1 Reef-Server
	s_sendmore(req,mode.c_str());
	s_send(req, aka.c_str());	//send wished alias
	
	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);
}

//sends a Message for publishing to the Reef-Server and expects a Message in return
bool ReefSatellite::pubAndReceive(RMessage& pub, RMessage& rec){
	s_sendmore(req, "3");
	s_sendmore(req, identity.c_str());
	s_sendmore(req, pub.getTags());
	s_send(req, pub.getBody());
	
	return receiveMessage(rec);
}

//sends a Message for publishing to the Reef-Server
void ReefSatellite::pub(RMessage& pub){
	s_sendmore(req, "2");
	s_sendmore(req, identity.c_str());
	s_sendmore(req, pub.getTags());
	s_send(req, pub.getBody());
	
	s_recv(req);//req has to receive rep to unblock, here empty	
}

//Sends a rec-Request to the Server, gets a stored Message in return
//returns false is no Message received
bool ReefSatellite::receive(RMessage& rec){
	s_sendmore(req, "4");
	s_send(req, identity.c_str());
	return receiveMessage(rec);
}

//receives a Message from the Server
//
bool ReefSatellite::receiveMessage(RMessage& repMsg){
	repMsg = RMessage();
	std::string numberMsgs;

	//receive Number of waiting Messages
	numberMsgs = s_recv(req);
	waitingMsgs = std::stoi(numberMsgs);

	//needed in Case Satellite needs to reregister at server, look at switch case -1 below
	std::string mode = "1";
	zmq::message_t reply;

	//if 0 Messages are waiting, non have been sent, return false
	switch (waitingMsgs){
		case -1: //NO QUEUE AT SERVER
			//reregister at Server			
			s_sendmore(req, mode.c_str());
			s_send(req, identity.c_str());			
			req.recv(&reply);
			return false;
			break;
		case 0:
			return false;
			break;
	}

	//  Process all parts of the message
	CJsonArray tags = jsonToArray(s_recv(req));
	std::string body = s_recv(req);

	//initiate the repMsg
	tagsInitMessage(repMsg, tags);
	tags.Clear();
	repMsg.initiateWithJson(body);

	//lower Number of Waiting Messages as 1 was just received
	waitingMsgs--;
	return true;	
}

//Initialize Message with Tags out of a CJsonArray
void ReefSatellite::tagsInitMessage(RMessage& msg, CJsonArray& array){
	for (unsigned i = 0; i < array.Size(); i++)
	{
		std::string tagtmp = array[i]->ToString();
		std::string tag = tagtmp.substr(1, tagtmp.size() - 2);
		msg.addTag(tag);
	}
}

/*
*	CJsonArray has not direct way to be parsed with a json-string
*	This method builds a CJsonObject with an CJsonArray as Member and then extracts the array
*/
CJsonArray ReefSatellite::jsonToArray(std::string arrayString){
	std::string objString = "{\"array\":" + arrayString + "}";	//the json string representing the object filled with an array
	CJsonObject* jsonObj = CJsonParser::Execute((jstring)objString); //parsing the json string
	CJsonArray jsonArray = CJsonArray(dynamic_cast<const CJsonArray*>((*jsonObj)["array"]));
	delete jsonObj;

	return &jsonArray;
}

//Offers possibility for the Satellite to react on to many waiting Messages at the Server
int ReefSatellite::getWaitingMsgs(){
	return waitingMsgs;
}

bool to_bool(std::string const& s) {
	return s != "0";
}

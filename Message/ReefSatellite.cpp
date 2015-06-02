#include "ReefSatellite.h"
#include <stdio.h>



ReefSatellite::ReefSatellite() :
context(1),
req(context, ZMQ_REQ)
{}


ReefSatellite::~ReefSatellite()
{
}

//returns 1 if everything worked as expected, other error codes will follow
int ReefSatellite::connect(std::string aka, std::string reefIp){
	identity = aka;
	connectRequest(aka, reefIp);
	return 1;
}

void ReefSatellite::connectRequest(std::string aka, std::string reefIp){
	std::string ip_str = "tcp://" + reefIp;
	std::string mode = "1";
	req.connect(ip_str.c_str());	//connect to 1 Server-Coral of Reef
	s_sendmore(req,mode.c_str());
	s_send(req, aka.c_str());	//send wished alias
	
	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);

	//TODO !!!!!!!!!!!!!!!!!!!
	//throw error if connection failed
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}


bool ReefSatellite::pubAndReceive(RMessage& pub, RMessage& rec){
	s_sendmore(req, "3");
	s_sendmore(req, identity.c_str());
	s_sendmore(req, pub.getTags());
	s_send(req, pub.getBody());
	
	return receiveMessage(rec);
}

void ReefSatellite::pub(RMessage& pub){
	s_sendmore(req, "2");
	s_sendmore(req, pub.getTags());
	s_send(req, pub.getBody());
	
	s_recv(req);//req has to receive rep to unblock, here empty	
}

bool ReefSatellite::receive(RMessage& rec){
	s_send(req, "4");
	s_sendmore(req, identity);
	return receiveMessage(rec);
}

bool ReefSatellite::receiveMessage(RMessage& repMsg){
	repMsg = RMessage();
	std::string numberMsgs;

	//receive Number of waiting Messages
	numberMsgs = s_recv(req);
	waitingMsgs = std::stoi(numberMsgs);

	//if 0 Messages are waiting, non have been sent, return false
	if (!waitingMsgs){
		return false;
	}

	//  Process all parts of the message
	CJsonArray tags = jsonToArray(s_recv(req));
	std::string body = s_recv(req);

	//initiate the repMsg
	tagsInitMessage(repMsg, tags);
	repMsg.initiateWithJson(body);

	//lower Number of Waiting Messages as 1 was just received
	waitingMsgs--;
	return true;	
}

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
const CJsonArray ReefSatellite::jsonToArray(std::string arrayString){
	std::string objString = "{\"array\":" + arrayString + "}";	//the json string representing the object filled with an array
	CJsonObject jsonObj = new CJsonObject(CJsonParser::Execute((jstring)objString)); //parsing the json string
	const CJsonArray* jsonArray = dynamic_cast<const CJsonArray*>(jsonObj["array"]);
	return jsonArray;
}

int ReefSatellite::getWaitingMsgs(){
	return waitingMsgs;
}

bool to_bool(std::string const& s) {
	return s != "0";
}

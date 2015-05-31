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
	req.connect(ip_str.c_str());	//connect to 1 Server-Coral of Reef
	s_send(req, aka.c_str());	//send wished alias
	
	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);

	//TODO !!!!!!!!!!!!!!!!!!!
	//throw error if connection failed
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}


bool ReefSatellite::pubAndReceive(RMessage& pub, RMessage& rec){
	s_sendmore(req, "Sys_PubAndReceive");
	s_sendmore(req, pub.getTags());
	s_send(req, pub.getBody());

	//TODO!!!!!!!!!!!!!!!!!!!!!
	//receive reply, check for RMessage, save in rec 
	//return true if received, false otherwise
	//!!!!!!!!!!!!!!!!!!!!!!!!!
	return true;
}

void ReefSatellite::pub(RMessage& pub){
	s_sendmore(req, "Sys_Pub");
	s_sendmore(req, pub.getTags());
	s_send(req, pub.getBody());
}

bool ReefSatellite::receive(RMessage& rec){
	s_send(req, "Sys_Rec");

	//TODO!!!!!!!!!!!!!!!!!!!!!
	//receive reply, check for RMessage, save in rec 
	//return true if received, false otherwise
	//!!!!!!!!!!!!!!!!!!!!!!!!!
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


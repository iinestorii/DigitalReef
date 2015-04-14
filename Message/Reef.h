//==================================================
//include guard
//==================================================
#ifndef __Reef_H_INCLUDED__
#define __Reef_H_INCLUDED__

//==================================================
// include dependencies
//==================================================
#include <cjson.h>
#include <RMessage.h>
#include <zhelpers.hpp>
//==================================================
// class declaration
//==================================================
class Reef
{
public:
	Reef();
	~Reef();
	int connect(std::string, std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	int initiate(std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	void addTag(std::string);
	void removeTag(std::string);
	void pubMessage(RMessage);
	RMessage receiveMessage();

private:
	CJsonObject adr_list;
	CJsonArray tag_list;
	zmq::context_t context;
	zmq::socket_t publisher;
	zmq::socket_t subscriber;
	zmq::socket_t req;
	zmq::socket_t rep;
	void connectRequest(std::string, std::string, std::string);
	void connectSubscribe();
};
#endif

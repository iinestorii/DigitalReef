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
#include <array>
//==================================================
// class declaration
//==================================================
class Reef
{
public:
	Reef();
	~Reef();
	int connect(std::string, std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	int initiate(std::string, std::string, std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	int initiate(std::string, std::string); //uses ports 5563 and 5565 as standards
	void addTag(std::string);
	void removeTag(std::string);
	void pubMessage(RMessage&);
	bool subMessage(RMessage&);
	
	void receiveMessage();

private:

	//Adress-List and Tag-List to keep track of Reef Members and Interests
	CJsonObject adr_list;
	std::string identity;
	std::vector<std::string> tag_list;

	//ZeroMQ context and sockets for the network
	zmq::context_t context;
	zmq::socket_t publisher;
	zmq::socket_t subscriber;
	zmq::socket_t req;
	zmq::socket_t rep;

	//ZeroMQ required attributes to check for new incoming Messages
	bool itemsSet=false;
	zmq::pollitem_t items[2];

	//Methods for ZeroMQ connectivity, test of Messages
	void itemsInit();
	bool findTag(std::string);
	void connectRequest(std::string, std::string, std::string);
	void connectSubscribe();
	const CJsonArray jsonToArray(std::string);
	void tagsInitMessage(RMessage& msg, CJsonArray& array);
	
};
#endif

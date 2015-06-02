//==================================================
//include guard
//==================================================
#ifndef __ReefSatellite_H_INCLUDED__
#define __ReefSatellite_H_INCLUDED__



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
class ReefSatellite
{
public:
	ReefSatellite();
	~ReefSatellite();
	int connect(std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	bool pubAndReceive(RMessage&, RMessage&);
	void pub(RMessage&);
	bool receive(RMessage&);
	int getWaitingMsgs();

private:

	std::string identity;
	unsigned int waitingMsgs=0; //Number of Messages in stack at Server waiting to be reveived by this Satellite
	//ZeroMQ context and sockets for the network
	zmq::context_t context;
	zmq::socket_t req;

	bool to_bool(std::string const&);
	bool receiveMessage(RMessage&);
	bool findTag(std::string);
	void connectRequest(std::string, std::string);
	const CJsonArray jsonToArray(std::string);
	void tagsInitMessage(RMessage& msg, CJsonArray& array);

};
#endif

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
#include <deque>
//==================================================
// class declaration
//==================================================
class ReefServer
{
public:
	ReefServer();
	~ReefServer();
	int connect(std::string, std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	int initiate(std::string, std::string, std::string, std::string); //returns 1 if everything worked as expected, other error codes will follow
	int initiate(std::string, std::string); //uses ports 5563 and 5565 as standards
	void addTag(std::string);
	void removeTag(std::string);
	void pubMessage(RMessage&);
	bool subMessage(RMessage&);



private:
	//struct needed to add Array to Map, as array has no copy-constructor
	struct satelliteMsgControl
	{
		int control[3]; //{position in satelliteMsgs, number of Msgs in the Queue, watchlist for deletion(false=0)}
	};

	//Adress-List and Tag-List to keep track of Reef Members and Interests
	CJsonObject adr_list; //adress-list of Reef-Servers
	std::string identity; //own alias in the Reef
	std::vector<std::string> tag_list; //own interests
	std::vector<std::string> satelliteAliases; //Aliases of dependend Satellites

	/*old satelliteMsgs, abondend because of copy constructor problems of CJsonArray in jsonplus lib
	std::vector< std::vector<RMessage> > satelliteMsgs; //Vector of Vector of Messages for each dependend Satellite
	*/

	// Vector of Deque of Messages for each dependend Satellite
	// satelliteMsgs[Satellite][ <tags>, <body> ]
	// Because of the deletion of the first halfe of the biggest Message Queue when MAX_MESSAGES is reached
	// a deque is used instead of a vector to enable efficient deletion at the beginning of the queue
	std::vector< std::deque<std::pair<std::string, std::string> > > satelliteMsgs;

	std::map<std::string, satelliteMsgControl> satMsgControlMap; //Control Numbers concerning the work on the satelliteMsgs

	//ZeroMQ context and sockets for the network
	zmq::context_t context;
	zmq::socket_t publisher;
	zmq::socket_t subscriber;
	zmq::socket_t req;
	zmq::socket_t rep;

	//ZeroMQ required attributes to check for new incoming Messages
	bool itemsSet = false;
	zmq::pollitem_t items[2];

	//Methods for ZeroMQ connectivity, test of Messages
	void itemsInit();
	bool findTag(std::string);
	void connectRequest(std::string, std::string, std::string);
	void connectSubscribe();
	CJsonArray jsonToArray(std::string);
	void tagsInitMessage(RMessage&, CJsonArray&);

	bool checkInterestAndProcess(RMessage&, std::string); //checks own and Satellite interest in RMessage and processes it acordingly
	bool receiveMsg(RMessage&);
	int getReceiveMsgMode();
	void newServer();
	void newSatellite();
	bool pubRequest(RMessage&);
	void recRequest(std::string);
	void saveMessage(std::string, std::pair<std::string, std::string>);

	//variables and methods to keep track and handling of message-queues
	const int MAX_MESSAGES = 20;
	int CUR_MESSAGES = 0;

	std::string checkForMaxMessages(); //returns true if entry got deleted
	std::string halveOrDeleteLongestQ();		   //returns true if entry got deleted
	void deleteSat(std::string, int);
	void resetWatchlist(std::string);

};
#endif
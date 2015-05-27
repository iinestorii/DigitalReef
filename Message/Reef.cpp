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
	std::cout << "nach connectRequest" << std::endl;
	//use the adr_list to connect your sub to all pubs in the reef
	connectSubscribe();

	return 1;
}

void Reef::connectRequest(std::string aka, std::string ownIp, std::string reefIp){
	std::string str_constructor = "tcp://" + reefIp;

	const char* ip_char = str_constructor.c_str();

	req.connect(ip_char);

	
	std::cout << "vor s_sendmore" << std::endl;
	s_sendmore(req, aka.c_str());
	std::cout << "vor s_send" << std::endl;
	s_send(req, ownIp.c_str());
	std::cout << "nach s_send" << std::endl;
	
	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);
	std::cout << "nach req.recv" << std::endl;
	//get the json string from the reply
	std::string json = std::string(static_cast<char*>(reply.data()), reply.size());

	std::cout << "nach jason parsen vorbereitung" << std::endl;
	std::cout << json << std::endl;
	//parse the json to a JsonObject and save it as the new adr_list
	adr_list = *CJsonParser::Execute((jstring)json);
	std::cout << "nach jason parsen " << std::endl;
}

//subscribes to each member of the address list adr_list
void Reef::connectSubscribe(){
	std::cout << "in connectSubscribe" << std::endl;

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
			subscriber.connect(address.c_str()); //HIER PROBLEM!!!!!!!!!!!!
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

	//test if there are incoming messages on subscriber or req socket
	//for now it waits 1 second (1000ms) before running along
	std::cout << "vor dem Block" << std::endl;
	zmq::poll(items, 2); 
	std::cout << "nach dem Block" << std::endl;
	std::cout << items[0].revents << std::endl;

	//resolve message at reply-socket
	if (items[0].revents & ZMQ_POLLIN) 
	{
		std::cout << "in Request part" << std::endl;
		/*TODO
		3. If Tag identifies as Pub-Request of Satellite-Coral, forward the Message, reply with stored incoming messages for Satellite Coral
		*/		

		//for now only handle new Reefmember requests
		receiveMessage();

		items[0].revents = 0;
	}
	//resolve message at subscribe-socket
	if (items[1].revents & ZMQ_POLLIN) 
	{
		std::cout << "in Subscribe part" << std::endl;
		// Convert to RMessage
		RMessage retVal;
		std::string builder = s_recv(subscriber);		
		retVal.initiateWithJson(builder);

		//TODO If Tag identifies as Message of interest for Satelite Coral store it and reply it with next request of Satellite

		
		items[1].revents = 0;

		//If Tag identifies as broadcast with new Member-info
		//add Info to adr_list
		//sub to pub of new Member
		if (retVal.containsAnyOf("SYS_newMember")){
			std::string aka = retVal.getString("aka");
			std::string adressListStr = retVal.getString("ip");
			adr_list.AddPare(aka, adressListStr);
			subscriber.connect(adressListStr.c_str());
		}

		//If Tag identifies as Message of interest for this Coral return it
		if (retVal.containsAnyOf(tag_list)){
			return &retVal;
		}
	}
	return 0;
}

/*
*	C++, especially in vs13 has some Problems with initializing arrays in the constructor
*	we don't want to initialize items[] everytime we poll, so the initialization got outsorced to 
*	here and the bool itemsSet is used to check befor each Poll if items[] is initialized
*/
void Reef::itemsInit(){
	items[0] = { rep, 0, ZMQ_POLLIN, 0 };
	items[1] = { subscriber, 0, ZMQ_POLLIN, 0 };
	itemsSet = true;
}

void Reef::receiveMessage(){
	std::vector<std::string> frames;
	
		zmq::message_t message;
		int more;               //  Multipart detection


		if (items[0].revents & ZMQ_POLLIN) {
			while (1) {
				//  Process all parts of the message
				rep.recv(&message);
				size_t more_size = sizeof(more);
				rep.getsockopt(ZMQ_RCVMORE, &more, &more_size);
				frames.push_back(std::string(static_cast<char*>(message.data()), message.size()));
				if (!more)
					break;      //  Last message part
			}
			for (unsigned n = 0; n<frames.size(); ++n) {
				std::cout << frames.at(n) << std::endl;
			}
		}

		//  Get new request from client, don't block if no Message is in queue

		// cheack if Message is received, if yes, proceed to add new Reef-Member
		std::string aka = frames.at(0);
		std::string ip = frames.at(1);
		adr_list.AddPare(aka, ip);
		frames.clear();

		//build RMessage with aka and ip to inform Reef about the new Member
		RMessage pubMember;
		pubMember.addSimplex("aka", aka);
		pubMember.addSimplex("ip", ip);
		pubMember.addTag("SYS_newMember");

		std::cout << "vor pubMessage(pubMember);" << std::endl;
		//publish the pubMember Message in the Reef
		pubMessage(pubMember);

		//reply to new Coral with adr_list
		std::string adr_list_str = adr_list.ToString();
		std::cout << adr_list_str << std::endl;
		/*
		//determine length of message for the char buffer
		int msg_length = adr_list_str.length();

		std::cout << "vor reply" << std::endl;
		//initialize message with specific length
		zmq::message_t reply(msg_length);

		memcpy((void *)reply.data(), adr_list_str.c_str(), msg_length);
		std::cout << "nach memcpy" << std::endl;
		//sends Message to new Coral
		rep.send(reply);

		*/
		s_send(rep, adr_list_str);
	
}
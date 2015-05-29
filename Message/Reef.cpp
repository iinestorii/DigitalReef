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

	

	s_sendmore(req, aka.c_str());

	s_send(req, ownIp.c_str());

	
	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);

	//get the json string from the reply
	std::string json = std::string(static_cast<char*>(reply.data()), reply.size());



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
			address = cvn->ToString(); //HIER PROBLEM! address kommt mit Ausführungszeichen. Zunächst nur Workaround.
			address = "tcp://"+address.substr(1, address.size() - 2);
			

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
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
	adr_list.AddPare(aka, adressListStr);
	return 1;
}

//returns 1 if everything worked as expected, other error codes will follow
int Reef::initiate(std::string aka, std::string ip){
	std::string adressListStr = ip + ":5563";
	publisher.bind("tcp://*:5563");
	rep.bind("tcp://*:5565");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
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
	s_sendmore(publisher, msg.getTags());
	s_send(publisher, msg.getBody());
}

//checks via tag_list if Message of interest has arrived
//returns pointer to interesting message
//returns null-pointer if no Message of interest was found
void Reef::subMessage(RMessage& retVal){
	//initialize items[] if it isn't already
	
	if (!itemsSet){
		itemsInit();
	}

	while (true){
		retVal = RMessage();
		//test if there are incoming messages on subscriber or req socket
		//for now it waits 1 second (1000ms) before running along	
		zmq::poll(items, 2, -1);


		//resolve message at reply-socket
		if (items[0].revents & ZMQ_POLLIN)
		{

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
			s_recv(subscriber); //empty envelop
			CJsonArray tags = jsonToArray(s_recv(subscriber)); //parse the json-String to CJsonArray
			std::string body = s_recv(subscriber);	//real content of message	

			retVal.initiateWithJson(body);
			
			tagsInitMessage(retVal, tags);
		
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
			} else if (retVal.containsAnyOf(tag_list)){ //If Tag identifies as Message of interest for this Coral return it
				break;
			}
		}
	}
	
}

void Reef::tagsInitMessage(RMessage& msg, CJsonArray& array){
	for (unsigned i = 0; i < array.Size(); i++)
	{
		std::string tagtmp = array[i]->ToString();
		std::string tag= tagtmp.substr(1, tagtmp.size() - 2);
		msg.addTag(tag);
	}
}

/*	
*	CJsonArray has not direct way to be parsed with a json-string
*	This method builds a CJsonObject with an CJsonArray as Member and then extracts the array
*/
const CJsonArray Reef::jsonToArray(std::string arrayString){
	std::string objString = "{\"array\":" + arrayString + "}";	//the json string representing the object filled with an array
	CJsonObject jsonObj = new CJsonObject(CJsonParser::Execute((jstring)objString)); //parsing the json string
	const CJsonArray* jsonArray = dynamic_cast<const CJsonArray*>(jsonObj["array"]);
	return jsonArray;
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


		if (items[0].revents & ZMQ_POLLIN) { //!!!!!!!!!!!!!IS THIS REALLY NEEDED?????????????????
			while (1) {
				//  Process all parts of the message
				rep.recv(&message);
				size_t more_size = sizeof(more);
				rep.getsockopt(ZMQ_RCVMORE, &more, &more_size);
				frames.push_back(std::string(static_cast<char*>(message.data()), message.size()));
				if (!more)
					break;      //  Last message part
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

		//publish the pubMember Message in the Reef
		pubMessage(pubMember);

		//reply to new Coral with adr_list
		std::string adr_list_str = adr_list.ToString();
		
		//sub to pub of new Coral
		ip =  "tcp://" + ip;
		std::cout << "IP Subscriber connects to:" << std::endl;
		std::cout << ip << std::endl;
		subscriber.connect(ip.c_str());
		std::cout << "subscribed!" << std::endl;
		s_send(rep, adr_list_str);
	
}
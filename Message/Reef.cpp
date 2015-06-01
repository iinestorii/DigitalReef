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
	identity = aka;
	//make a Request to connect to the reef, saves Reef Addresslist in adr_list
	connectRequest(aka, ownIp, reefIp);

	//use the adr_list to connect your sub to all publisher in the reef
	std::cout << "vor connectSubscribe" << std::endl;
	connectSubscribe();
	std::cout << "nach connectSubscribe" << std::endl;
	return 1;
}

void Reef::connectRequest(std::string aka, std::string ownIp, std::string reefIp){
	std::string ip_str = "tcp://" + reefIp;
	req.connect(ip_str.c_str());	//connect to 1 Server-Coral of Reef
	s_sendmore(req, aka.c_str());	//send wished alias
	s_send(req, ownIp.c_str());		//send adress of own publisher
		
	//receive the reply
	zmq::message_t reply;
	req.recv(&reply);

	//get the json-string with adress-list of Reef-Corals from the reply
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
	keys.erase(std::remove(keys.begin(), keys.end(), identity),keys.end()); //erase own aka to avoid subscribing to own publisher
	std::vector<jstring>::iterator it;
	int connectError = 0;

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
	identity = aka;
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
	identity = aka;
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


//checks	via tag_list if Message of interest has arrived and saves it
//			for System Messages that need to be taken care of
//param		RMessage&	in which Message of interest will be safed if found
//returns	false		if no Message of interest was found
//			true		if Message of interest was found
bool Reef::subMessage(RMessage& retVal){
	
	//initialize items[] if it isn't already	
	if (!itemsSet){
		itemsInit();
	}
	
	bool retBool = false;
	bool queue_Not_Empty = true;

	//loops until all queued Messages are worked off or a Message of interest gets returned (return true;)
	while (queue_Not_Empty){

		queue_Not_Empty = false;
		retVal = RMessage();

		//test if there are incoming messages on subscriber or req socket
		//for now it does not block if there is no Message	
		zmq::poll(items, 2, 0);


		//resolve message at reply-socket
		if (items[0].revents & ZMQ_POLLIN)
		{
			queue_Not_Empty = true;
			receiveMsg();
			items[0].revents = 0;
		}

		//resolve message at subscribe-socket
		if (items[1].revents & ZMQ_POLLIN)
		{
	
			queue_Not_Empty = true;

			s_recv(subscriber);									//empty envelop
			CJsonArray tags = jsonToArray(s_recv(subscriber));	//parse the json-String to CJsonArray
			std::string body = s_recv(subscriber);				//real content of message	
			
			tagsInitMessage(retVal, tags);
		
			//TODO If Tag identifies as Message of interest for Satelite Coral store it and reply it with next request of Satellite
			items[1].revents = 0;

			//If Tag identifies as broadcast with new Member-info
			//add Info to adr_list
			//sub to pub of new Member
			if (retVal.containsAnyOf("SYS_newMember")){
				retVal.initiateWithJson(body);
				std::string aka = retVal.getString("aka").substr(1, retVal.getString("aka").size() - 2);
				std::string adressListStr = retVal.getString("ip").substr(1, retVal.getString("ip").size() - 2);
				adr_list.AddPare(aka, adressListStr);
				std::string adressConnectStr = "tcp://" + adressListStr;
				std::cout << adressConnectStr << std::endl;
				subscriber.connect(adressConnectStr.c_str());
				
			} else {
				if (retVal.containsAnyOf(tag_list)){ //If Tag identifies as Message of interest for this Coral return it
					retVal.initiateWithJson(body);
					queue_Not_Empty = false;
					retBool = true;
				}
				if (retVal.containsAnyOf(satelliteAliases)){ //If Tag identifies as Message of interest for at least one dependent Satellite
					retVal.initiateWithJson(body);

					for (std::vector<std::string>::iterator it = satelliteAliases.begin(); it != satelliteAliases.end(); ++it) {
						if (retVal.containsAnyOf(*it)){
							saveMessage(*it, retVal);
						}
					}
				}
			}
		}
	}

	if (!retBool)retVal = RMessage(); //clear Message
	return retBool; //no message of interest found
}

void Reef::saveMessage(std::string aka, RMessage& msg){
	//find satellite by alias in the map
	auto search = satMsgControlMap.find(aka);

	if (search != satMsgControlMap.end()) { //if sat has been found

		//get the Controlnumbers for this satellite
		satelliteMsgControl msgControl = search->second;
		int satPosition = msgControl.control[0];

		//insert msg at the back of the queue for this satellite
		satelliteMsgs[satPosition].push_back(msg);

		//add 1 to the Number of Messages in this queue
		search->second.control[1]++;
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

void Reef::receiveMsg(){
	int msgMode = getReceiveMsgMode();
	std::string aka;
	switch (msgMode){
	case 0: //add new Server to the Reef
		newServer();
		break;
	case 1: //add new Satellite to the Reef
		newSatellite();
		break;
	case 2: //publish Messages for a dependent Satellite
		pubRequest();
		break;
	case 3: //publish Messages for a dependent Satellite, reply with stored Messages for Satellite
		 aka = s_recv(rep);		
		pubRequest();
		recRequest(aka);
		break;
	case 4: //reply with stored Messages for Satellite
		aka = s_recv(rep);
		recRequest(aka);
		break;
	default: //TODO Throw Error?
		break;
	}	
}

//checks the first Frame of a received Message for the intended Mode of the sender
int Reef::getReceiveMsgMode(){
	zmq::message_t message;
	rep.recv(&message);
	std::string msgModeStr = std::string(static_cast<char*>(message.data()), message.size());
	return std::stoi(msgModeStr);
}

//handles the connection request of a new server coral
void Reef::newServer(){

	std::vector<std::string> frames;
	zmq::message_t message;
	int more;               //  Multipart detection

	while (true) {
		//  Process all parts of the message
		rep.recv(&message);
		size_t more_size = sizeof(more);
		rep.getsockopt(ZMQ_RCVMORE, &more, &more_size);
		frames.push_back(std::string(static_cast<char*>(message.data()), message.size()));
		if (!more)
			break;      //  Last message part
	}

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
	ip = "tcp://" + ip;
	subscriber.connect(ip.c_str());
	s_send(rep, adr_list_str);

}
// handles the connection request of a new satellite coral
// this coral will be dependent of this server
void Reef::newSatellite(){
	zmq::message_t message;

	//Receive and save alias of satellite
	rep.recv(&message);
	std::string aka = std::string(static_cast<char*>(message.data()), message.size());
	satelliteAliases.push_back(aka);

	//Add MessageQueue for Satellite
	std::vector<RMessage> msgQueue;
	satelliteMsgs.push_back(msgQueue);

	//Add MessageQueue-Controls for Satellite
	satelliteMsgControl controls{ { satelliteMsgs.size() - 1, 0 } };
	satMsgControlMap.insert(std::pair<std::string, satelliteMsgControl> (aka, controls));

	//Answer Satellite with 1 for confirmation
	s_send(rep, "1");
}

// pubRequest by one of the Satellites, this Server just forwards it by publishing it
void Reef::pubRequest(){

	//TODO!!!!!!!!!!!!!!!!!!
	// Check if msg is of interest for this server or other dependent satellites
	//!!!!!!!!!!!!!!!!!!!!!!
	zmq::message_t message;
	
	//send empty envelope
	s_sendmore(publisher, "");

	//receive tag-part of message and publish it
	rep.recv(&message);
	zmq_send(publisher, &message, message.size(), ZMQ_SNDMORE);
	//receive body-part of message and publish it
	rep.recv(&message);
	zmq_send(publisher, &message, message.size(), 0);
}

//recRequest by one of the Satellites, reply with number of stored msgs and the oldest stored msg
void Reef::recRequest(std::string aka){	
	
	//find satellite by alias in the map
	auto search = satMsgControlMap.find(aka);

	if (search != satMsgControlMap.end()) { //if sat has been found

		//get the Controlnumbers for this satellite
		satelliteMsgControl msgControl = search->second;
		int msgCount = msgControl.control[1]; //number of stored Messages for this Satellite
		s_sendmore(rep, std::to_string(msgCount)); //send this number to the satellite
		if (!msgCount){ 
			int msgPosition = msgControl.control[0]; //find position of Message Queue in the vector of all Queues
			RMessage msg = satelliteMsgs[msgPosition][0]; //get the oldest Message
			s_sendmore(rep, msg.getTags()); //send the Message to the satellite
			s_send(rep, msg.getBody());
			satelliteMsgs[msgPosition].erase(satelliteMsgs[msgPosition].begin()); //erase the Message from the Queue
			msgControl.control[1]--; //lower the count of stored Messages by 1
		}

	}else{
		//TODO!!!!!!!!!!!!!!!!!!!!
		//What happens if there is no entry for the Satellite in the map?
		//!!!!!!!!!!!!!!!!!!!!!!!!
	}


}
#include "ReefServer.h"
#include <stdio.h>



ReefServer::ReefServer() :
	context(1),
	publisher(context, ZMQ_PUB),
	subscriber(context, ZMQ_SUB),
	req(context, ZMQ_REQ),
	rep(context, ZMQ_REP)
{}


ReefServer::~ReefServer()
{
}
//returns 1 if everything worked as expected, other error codes will follow
int ReefServer::connect(std::string aka, std::string ownIp, std::string reefIp){
	identity = aka;
	//make a Request to connect to the reef, saves Reef Addresslist in adr_list
	connectRequest(aka, ownIp, reefIp);

	//use the adr_list to connect your sub to all publisher in the reef
	connectSubscribe();
	return 1;
}

void ReefServer::connectRequest(std::string aka, std::string ownIp, std::string reefIp){
	std::string ip_str = "tcp://" + reefIp;
	req.connect(ip_str.c_str());	//connect to 1 Server-Coral of Reef
	s_sendmore(req, "0");
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
void ReefServer::connectSubscribe(){
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
int ReefServer::initiate(std::string aka, std::string ip, std::string pubPort, std::string repPort){
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
int ReefServer::initiate(std::string aka, std::string ip){
	identity = aka;
	std::string adressListStr = ip + ":5563";
	publisher.bind("tcp://*:5563");
	rep.bind("tcp://*:5565");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);
	adr_list.AddPare(aka, adressListStr);
	return 1;
}

//returns true if tag_list contains tag
bool ReefServer::findTag(std::string tag){
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
void ReefServer::addTag(std::string tag){
	if (!findTag(tag)){
		tag_list.push_back(tag);
	}	
}

//removes a tag of the tag_list
void ReefServer::removeTag(std::string tag){
	tag_list.erase(std::remove(tag_list.begin(), tag_list.end(), tag), tag_list.end());
}

void ReefServer::pubMessage(RMessage& msg){
	s_sendmore(publisher, "");
	s_sendmore(publisher, msg.getTags());
	s_send(publisher, msg.getBody());
}


//checks	via tag_list if Message of interest has arrived and saves it
//			for System Messages that need to be taken care of
//			for Satellite Messages that need to be routed
//param		RMessage&	in which Message of interest will be safed if found
//returns	false		if no Message of interest was found
//			true		if Message of interest was found
bool ReefServer::subMessage(RMessage& retVal){
	
	//initialize items[] if it isn't already	
	if (!itemsSet){
		itemsInit();
	}
	
	bool retBool = false;
	bool queue_Not_Empty = true;

	//loops until all queued Messages are worked off and no Message of interest got returned (return true;)
	while (queue_Not_Empty && !retBool){

		queue_Not_Empty = false;

		retVal.clear();

		//test if there are incoming messages on subscriber or req socket
		//for now it does not block if there is no Message	
		zmq::poll(items, 2, 0);


		//resolve message at reply-socket
		if (items[0].revents & ZMQ_POLLIN)
		{
			//queue was not empty
			queue_Not_Empty = true;

			//receive Message on Rep-Port
			//If the Message is from pub-request of a satellite
			//connected to this server and the Message has at least one
			//Tag that interests this server this will return true
			retBool=receiveMsg(retVal);

			//reset poll for rep-port
			items[0].revents = 0;
		}

		//resolve message at subscribe-socket
		if (!retBool && (items[1].revents & ZMQ_POLLIN))
		{	
			//queue was not empty
			queue_Not_Empty = true;

			s_recv(subscriber);	//empty envelop	
			std::string tagsStr = s_recv(subscriber);		//tags of message
			CJsonArray tagsArray = jsonToArray(tagsStr);	//parse the json-String to CJsonArray
			std::string bodyStr = s_recv(subscriber);		//real content of message	
			
			//parse the tags of the message and check for own and satellite interest
			//body parsed only if needed
			tagsInitMessage(retVal, tagsArray);
		
		
			//reset poll for sub-port
			items[1].revents = 0;

			//If Tag identifies as broadcast with new Member-info
			//add Info to adr_list
			//sub to pub of new Member
			if (retVal.containsAnyOf("SYS_newMember")){

				//parse the body of the Message
				retVal.initiateWithJson(bodyStr);

				//get the name and the ip of the new reef-server
				std::string aka = retVal.getString("aka").substr(1, retVal.getString("aka").size() - 2);
				std::string adressListStr = retVal.getString("ip").substr(1, retVal.getString("ip").size() - 2);

				//add name and ip of new reef-server to adr_list
				adr_list.AddPare(aka, adressListStr);

				//subscribe to pub of new reef-server
				std::string adressConnectStr = "tcp://" + adressListStr;
				subscriber.connect(adressConnectStr.c_str());
				
			} else { //check Message-Tags for own interest or interest of satellites
				retBool = checkInterestAndProcess(retVal, bodyStr);
			}
		tagsArray.Clear();
		}
	}

	//clear Message retVal if no Message if interest was found
	//so that the user can't get hold of any system messages by accident
	if (!retBool)retVal.clear();

	//return true if Message of interest has been found
	return retBool; 
}

//checks	interest of this server in Message, returns true if interested
//			interest of Satellites dependent of this Server, saves message if interested
bool ReefServer::checkInterestAndProcess(RMessage& msg, std::string body){
	bool retBool = false;

	//If Tag identifies as Message of interest for this Coral return it
	if (msg.containsAnyOf(tag_list)){ 
		msg.initiateWithJson(body);
		retBool = true;
	}

	//If Tag identifies as Message of interest for at least one dependent Satellite
	//save it in the Msgqueue for each Satellite with interest
	//test if MAX_MESSAGES is reached and delete Messages/Queue if necessary
	if (msg.containsAnyOf(satelliteAliases)){ 
		//the messages that will be sent
		std::pair<std::string, std::string> msgPair = std::make_pair(msg.getTags(), body);

		//each time the message is saved in one queue it will be checked if MAX_QUEUE is reached
		//if there is the need to delete a complete queue its name will be saved in this vector to be deleted
		//in satelliteAliases later, as we iterate over satelliteAliases and don't want to delete elements of it during the iteration
		std::vector<std::string> toBeDeleted;
		
		for (std::vector<std::string>::iterator it = satelliteAliases.begin(); it != satelliteAliases.end(); ++it) {

			//if a tag of the message corresponds with the current aka in satelliteAlliases
			if (msg.containsAnyOf(*it)){
				
				//check for MAX_MESSAGES reached, returns empty string if no Queue had to be deleted
				//otherwise returns name of queue that was deleted
				std::string tmpAKA = checkForMaxMessages();

				//if tmpAKA is not empty, save the name of the deleted Queue in toBeDeleted
				if(!tmpAKA.empty())toBeDeleted.push_back(tmpAKA);

				//save the new Message in the q of the current SatelliteAlias
				//saveMessage ignored calls to deleted Queues
				saveMessage(*it, msgPair);	
			}
		}

		//finally delete the Aliases of the Queues that have been deleted in satAliases
		/*
		NOTE: This could result in horrible performance in WORST CASE, however it is really unlikely.
			WORST CASE: 1.The Server has almost as many dependend Satellites (or even more) as MAX_MESSAGES 
						2.CUR_MESSAGES is almost as high as MAX_MESSAGES
						3.The Message has to be saved for (almost) all Satellites
						4.All satellites were already on the watchlist

			Then there would me many entries in the toBeDeleted vector, resulting in many 
			satelliteAliases.erase(...) calls, each of which would result in copying all entries after the erased entry

			NORMAL CASE: The Network is designed in a fashion that under normal circumstances 
						1.the MAX_MESSAGES won't be reached
						2.if the MAX_MESSAGES gets reached, the satellites have at leas enough time to get of the watchlist
						3.there are not that many recipience of a Message, that a complete Deletion of the longest Queue 
						still doesn't give enough space
			In the NORMAL CASE toBeDeleted should have a maximum of 1 Entry, resulting in only 1 erase in satelliteAliases.
			toBeDeleted is only a Vector to be secure even in WORST CASE
		*/
		for (std::vector<std::string>::iterator it = toBeDeleted.begin(); it != toBeDeleted.end(); ++it) {
			satelliteAliases.erase(std::remove(satelliteAliases.begin(), satelliteAliases.end(), *it), satelliteAliases.end());
		}
	}
	return retBool;
}

std::string ReefServer::checkForMaxMessages(){
	if (CUR_MESSAGES >= MAX_MESSAGES){
		return halveOrDeleteLongestQ();
	}
	else return "";
}

std::string ReefServer::halveOrDeleteLongestQ(){

	//Iterator to find longest Queue in satMsgsControlMap
	std::map<std::string, satelliteMsgControl>::iterator iter;

	std::string returnValue = "";
	//variables to safe information of longest Queue
	std::string longestQ;
	bool watchlist = false;
	int qPosition=-1;
	int qLength=-1;

	//finds the longest Message Queue and saves its information
	for (iter = satMsgControlMap.begin(); iter != satMsgControlMap.end(); iter++){
		if (iter->second.control[1] > qLength){
			qPosition = iter->second.control[0];
			qLength = iter->second.control[1];
			watchlist = iter->second.control[2];
			longestQ = iter->first;
		}
	}

	//variable to control amount of deleted Messages
	int deleteAmount = 0;

	//if the longestQueue already is on the watchlist, it will be deleted completely, not halved
	//it is on the watchlist if it had to be halved before and the owning Satellite hasn't contacted this Server since
	//in this case a disconnect by the satellite is asumend and his entries on this server deleted
	if (watchlist){
		deleteSat(longestQ, qPosition); //delete all entries concerning this Satellite
		deleteAmount = qLength; //in this case the number of deleted Messages equals the lenght of the Queue
		returnValue = longestQ;
	}
	//otherwise delete only the first halve (the oldest) of the Messages for this Satellite
	else if (qPosition != -1){
		deleteAmount = qLength / 2; //delete amount is halve of the Queue length

		//erase the first half of satelliteMsgs[qPosition], which is a deque 
		//to handle erasing the first entries more efficiently than a vector
		satelliteMsgs[qPosition].erase(satelliteMsgs[qPosition].begin(), 
			satelliteMsgs[qPosition].begin() + deleteAmount);

		//update the controlMap of the Satellite with the new Queue Length and place it on the watchlist
		satMsgControlMap[longestQ].control[1] -= deleteAmount;
		satMsgControlMap[longestQ].control[2] = true; //watchlist
	}
	CUR_MESSAGES -= deleteAmount; //adjust the number of current Messages
	return returnValue;
}

//deletes Satellite aka with satelliteMsgs-Position qPosition and updates other Queues about Position change
void ReefServer::deleteSat(std::string aka, int qPosition){
	std::map<std::string, satelliteMsgControl>::iterator iter;
	//erase the Satellite out of the queue-handling vectors
	satMsgControlMap.erase(aka);
	satelliteMsgs.erase(satelliteMsgs.begin() + qPosition);
	
	//each Satellite with a higher qPosition then the deleted one has to adjust his Position by -1
	for (iter = satMsgControlMap.begin(); iter != satMsgControlMap.end(); iter++){
		if (iter->second.control[0] > qPosition) iter->second.control[0]--;
	}
	std::cout << "Sat deleted" << std::endl;
}

void ReefServer::saveMessage(std::string aka, std::pair<std::string, std::string> msg){
	//find satellite by alias in the map
	auto search = satMsgControlMap.find(aka);
	
	if (search != satMsgControlMap.end()) { //if sat has been found

		//get the Controlnumbers for this satellite
		satelliteMsgControl& msgControl = search->second;
		
		int satPosition = msgControl.control[0];
		
		//insert msg at the back of the queue for this satellite
		satelliteMsgs[satPosition].emplace_back(msg);
		
		//add 1 to the Number of Messages in this queue
		search->second.control[1]++;

		//add 1 to the sum of Messages in all queues
		CUR_MESSAGES++;

		}
}



void ReefServer::tagsInitMessage(RMessage& msg, CJsonArray& array){
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
CJsonArray ReefServer::jsonToArray(std::string arrayString){
	std::string objString = "{\"array\":" + arrayString + "}";	//the json string representing the object filled with an array
	CJsonObject* jsonObj = CJsonParser::Execute((jstring)objString); //parsing the json string
	CJsonArray jsonArray = CJsonArray(dynamic_cast<const CJsonArray*>((*jsonObj)["array"]));
	delete jsonObj;

	return &jsonArray;
}


/*
*	C++, especially in vs13 has some Problems with initializing arrays in the constructor
*	we don't want to initialize items[] everytime we poll, so the initialization got outsorced to 
*	here and the bool itemsSet is used to check befor each Poll if items[] is initialized
*/
void ReefServer::itemsInit(){
	items[0] = { rep, 0, ZMQ_POLLIN, 0 };
	items[1] = { subscriber, 0, ZMQ_POLLIN, 0 };
	itemsSet = true;
}

bool ReefServer::receiveMsg(RMessage& msg){
	bool retBool = false;
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
		aka = s_recv(rep);
		resetWatchlist(aka);
		retBool=pubRequest(msg);
		s_send(rep, ""); //request socket of the satellite needs a reply to unblock
		break;
	case 3: //publish Messages for a dependent Satellite, reply with stored Messages for Satellite
		 aka = s_recv(rep);	
		 resetWatchlist(aka);
		 retBool = pubRequest(msg);
		//std::cout << "vor recRequest case 3" << std::endl;
		recRequest(aka);
		//std::cout << "nach recRequest" << std::endl;
		break;
	case 4: //reply with stored Messages for Satellite
		aka = s_recv(rep);
		resetWatchlist(aka);
		//std::cout << "vor recRequest case 4" << std::endl;
		recRequest(aka);
		//std::cout << "nach recRequest" << std::endl;
		break;
	default: //TODO Throw Error?
		break;
	}	
	return retBool;
}

//sets the Watchlistentry of a Satellite to false
void ReefServer::resetWatchlist(std::string aka){
	auto search = satMsgControlMap.find(aka);

	if (search != satMsgControlMap.end()) { //if sat has been found
		search->second.control[2] = false;
	}

}

//checks the first Frame of a received Message for the intended Mode of the sender
int ReefServer::getReceiveMsgMode(){
	zmq::message_t message;
	rep.recv(&message);
	std::string msgModeStr = std::string(static_cast<char*>(message.data()), message.size());
	return std::stoi(msgModeStr);
}

//handles the connection request of a new server coral
void ReefServer::newServer(){

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

//handles the connection request of a new satellite coral
//the satellite will be dependent of this server
//NOTICE if there has already been added a satellite with the same alias, the server will assume
//that	either the satellite got disconnected and now reconnects
//		or it's a second satellite and all satellites with the same alias act as workers, 
//		reveiving new Messages when they are ready
//either way the server will not add a new queue for them, but give them access to the old message queue
//which will be shared from this point onwards between all workers
//CAUTION!!! This behaviour changes if workers connect to different servers
//		1 Message will be worked on by 1 Worker per Server, so multiple workers could end up working on the same Message
//while this behaviour offends the location independency, synching multiple servers to avoid it would result in 
//disproportional overhead
//this implementation allows the utilization of a simple worker structure without much overhead
void ReefServer::newSatellite(){
	zmq::message_t message;

	//Receive and save alias of satellite
	rep.recv(&message);
	std::string aka = std::string(static_cast<char*>(message.data()), message.size());

	//if there is no satellite with alias aka yet, add it
	if (!(std::find(satelliteAliases.begin(), satelliteAliases.end(), aka) != satelliteAliases.end())) {
		satelliteAliases.push_back(aka);

		//Add MessageQueue for Satellite
		std::deque<std::pair<std::string, std::string> > msgQueue;
		satelliteMsgs.push_back(msgQueue);

		//Add MessageQueue-Controls for Satellite
		satelliteMsgControl controls{ { satelliteMsgs.size() - 1, 0, 0 } };
		satMsgControlMap.insert(std::make_pair(aka, controls));
	}
	//Answer Satellite with 1 for confirmation
	s_send(rep, "1");
}

// pubRequest by one of the Satellites, this Server just forwards it by publishing it
bool ReefServer::pubRequest(RMessage& msg){
	msg = RMessage();
	bool retBool;
	std::string tagsStr = s_recv(rep); //receive message tags
	std::string bodyStr = s_recv(rep); //receive messagebody
	CJsonArray tagsArray = jsonToArray(tagsStr); //parse Tags
	tagsInitMessage(msg, tagsArray); //initiate msg with the tags
	tagsArray.Clear();
	
	retBool = checkInterestAndProcess(msg, bodyStr);
	s_sendmore(publisher, ""); //send empty envelope
	s_sendmore(publisher, tagsStr);	//receive tag-part of message and publish it
	s_send(publisher, bodyStr); //receive body-part of message and publish it
	return retBool;
}

//recRequest by one of the Satellites, reply with number of stored msgs and the oldest stored msg
void ReefServer::recRequest(std::string aka){
	//find satellite by alias in the map
	auto search = satMsgControlMap.find(aka);

	if (search != satMsgControlMap.end()) { //if sat has been found
		//get the Controlnumbers for this satellite
		satelliteMsgControl& msgControl = search->second;
		int msgCount = msgControl.control[1]; //number of stored Messages for this Satellite

		if (!msgCount){
			s_send(rep, std::to_string(msgCount)); //send the number of messages to the satellite
		}
		else{
			s_sendmore(rep, std::to_string(msgCount)); //send the number of messages to the satellite			
			int msgPosition = msgControl.control[0]; //find position of Message Queue in the vector of all Queues

			std::pair<std::string, std::string> msg = satelliteMsgs[msgPosition][0];

			s_sendmore(rep, msg.first); //send the Message to the satellite
			s_send(rep, msg.second);

			satelliteMsgs[msgPosition].erase(satelliteMsgs[msgPosition].begin()); //erase the Message from the Queue
			msgControl.control[1]--; //lower the count of stored Messages by 1	
			CUR_MESSAGES--;
		}
	}else{
		s_send(rep, "-1");
	}


}
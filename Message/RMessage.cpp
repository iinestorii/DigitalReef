#include <RMessage.h>
#include <iostream>

RMessage::RMessage(void){
	
}

RMessage::RMessage(const RMessage * msg){
	tags=(RMessageTags&)msg->tags;
	body = msg->body;
}


RMessage::~RMessage(void){
	clear();
}

int RMessage::addSimplex(std::string key, char * value){
	return body.addSimplex(key, value);
}

int RMessage::addSimplex(std::string key, int value){
	return body.addSimplex(key, value);
}


int RMessage::addSimplex(std::string key, std::string value){

	return body.addSimplex(key, value);
}

int RMessage::addSimplex(std::string key, bool value){

	return body.addSimplex(key, value);
}


int RMessage::addArray(std::string key, RMessageArray& array){
	return body.addArray(key, array);
}

int RMessage::addInnerBody(std::string key, RMessageBody & innerBody){
	return body.addInnerBody(key, innerBody);
}

void RMessage::initiateWithJson(std::string json){
	body.initiateWithJson(json);
}

std::string RMessage::getBody(){
	return body.getObject().ToString();
}

std::string RMessage::getString(std::string key){
	return body.getString(key);
}

std::string RMessage::getString(char* key){
	return body.getString(key);
}

int RMessage::getInt(std::string key){
	return body.getInt(key);
}

int RMessage::getInt(char* key){
	return body.getInt(key);
}

bool RMessage::getBool(std::string key){
	return body.getBool(key);
}

bool RMessage::getBool(char* key){
	return body.getBool(key);
}


RMessageArray& RMessage::getArray(std::string key){
	return body.getArray(key);
}


RMessageArray& RMessage::getArray(char* key){
	return body.getArray(key);
}


RMessageBody& RMessage::getInnerBody(std::string key){
	return body.getInnerBody(key);
}

RMessageBody& RMessage::getInnerBody(char* key){
	return body.getInnerBody(key);
}

int RMessage::addTag(std::string tag){
	return tags.addTag(tag);
}

int RMessage::addTag(char* tag){
	return tags.addTag(tag);
}

void RMessage::initiateTagsWithArray(CJsonArray* jsonArray){
	tags.setArray(*jsonArray);
}

bool RMessage::containsAnyOf(char* input_tag){
	return tags.containsAnyOf(input_tag);
}

bool RMessage::containsAnyOf(std::string input_tag){
	return tags.containsAnyOf(input_tag);
}

bool RMessage::containsAnyOf(std::vector<std::string> input_tags){
	return tags.containsAnyOf(input_tags);
}

std::string RMessage::getTags(){
	return tags.getTags();
}

void RMessage::clear(){
	body.clear();
	tags.clear();
}
#include <RMessage.h>
#include <iostream>

RMessage::RMessage(void){
	
}


RMessage::~RMessage(void){

}

void RMessage::addSimplex(std::string key, char * value){
	body.addSimplex(key, value);
}

void RMessage::addSimplex(std::string key, int value){
	body.addSimplex(key, value);
}


void RMessage::addSimplex(std::string key, std::string value){

	body.addSimplex(key, value);
}

void RMessage::addSimplex(std::string key, bool value){

	body.addSimplex(key, value);
}


void RMessage::addArray(std::string key, RMessageArray& array){
	body.addArray(key, array);
}

void RMessage::addInnerBody(std::string key, RMessageBody & innerBody){
	body.addInnerBody(key, innerBody);	
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

void RMessage::addTag(std::string tag){
	tags.addTag(tag);
}

void RMessage::addTag(char* tag){
	tags.addTag(tag);
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
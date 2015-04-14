#include "RMessageBody.h"
#include "RMessageArray.h"
#include <iostream>
#include <cjsonvalue_common.h>

RMessageBody::RMessageBody(void){
}


RMessageBody::~RMessageBody(void){
	}


void RMessageBody::addSimplex(std::string key, char * value){
	json_object.AddPare(key, value);
}

void RMessageBody::addSimplex(std::string key, int value){
	json_object.AddPare(key, (jint)value);
}

void RMessageBody::addSimplex(std::string key, std::string value){

	json_object.AddPare(key, value);
}

void RMessageBody::addSimplex(std::string key, bool value){
	json_object.AddPare(key, value);
}

void RMessageBody::addInnerBody(std::string key, RMessageBody  &innerbody){
	CJsonObject  &innerObject = innerbody.getObject();

	json_object.AddPare(key, &innerObject);

}


/*/////////////////////////////////////////////////
***	returns Reference to the json:Object		***
*//////////////////////////////////////////////////
CJsonObject RMessageBody::getObject(){
	return &json_object;
}

void RMessageBody::initiateWithJson(std::string json){
	json_object = *CJsonParser::Execute((jstring)json);
}

/*/////////////////////////////////////////////////
***	Adds the CJsonArray of value to the body	***
***	value.getArray() returns Reference to the	***
***	Array, not a copy							***
*//////////////////////////////////////////////////
void RMessageBody::addArray(std::string key, RMessageArray& value){	
	 json_object.AddPare(key, &value.getArray());
}
void RMessageBody::setJsonObject(std::string json){
	json_object = new CJsonObject(CJsonParser::Execute((jstring)json));
	std::cout << "RMessageBody.setJsonObject() durchgefuehrt" << std::endl;
	std::cout << json_object.ToString() << std::endl;
}


std::string RMessageBody::toString(){
	std::string tmp = json_object.ToString();	
	return tmp;
}

std::string RMessageBody::getString(std::string key){

	if (json_object[key] && json_object[key]->GetType() == 0) //There is an object with name key and it is a CJsonValueNumber
	{
		const CJsonValue* cvn = json_object[key];
		std::string result;
		result = cvn->ToString();
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

std::string RMessageBody::getString(char* key){

	if (json_object[key] && json_object[key]->GetType() == 0) //There is an object with name key and it is a CJsonValueNumber
	{
		const CJsonValue* cvn = json_object[key];
		std::string result;
		result=cvn->ToString();
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

int RMessageBody::getInt(std::string key){

	if (json_object[key] && json_object[key]->GetType() == 1) //There is an object with name key and it is a CJsonValueNumber
	{	
		// dynamic_cast used as CJsonValueNumber*->getValue is not 
		// a virtual method and can't be reached with 
		// CJsonValueNumber, alternatively there could have been made some changes
		// in the jsonplus.lib
		const CJsonValueNumber* cvn = dynamic_cast<const CJsonValueNumber*>(json_object[key]);
		int result;
		cvn->GetValue(result);
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

int RMessageBody::getInt(char* key){

	if (json_object[key] && json_object[key]->GetType() == 1) //There is an object with name key and it is a CJsonValueNumber
	{
		// dynamic_cast used as CJsonValueNumber*->getValue is not 
		// a virtual method and can't be reached with 
		// CJsonValueNumber, alternatively there could have been made some changes
		// in the jsonplus.lib
		const CJsonValueNumber* cvn = dynamic_cast<const CJsonValueNumber*>(json_object[key]);
		int result;
		cvn->GetValue(result);
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

bool RMessageBody::getBool(char* key){

	if (json_object[key] && json_object[key]->GetType() == 5) //There is an object with name key and it is a CJsonValueNumber
	{
		const CJsonValue* cvn = json_object[key];
		bool result;
		std::istringstream(cvn->ToString()) >> std::boolalpha >> result;
		result;
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

bool RMessageBody::getBool(std::string key){

	if (json_object[key] && json_object[key]->GetType() == 5) //There is an object with name key and it is a CJsonValueNumber
	{
		const CJsonValue* cvn = json_object[key];
		bool result;
		std::istringstream(cvn->ToString()) >> std::boolalpha >> result;
		result;
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

RMessageArray& RMessageBody::getArray(char* key){
	if (json_object[key] && json_object[key]->GetType() == 3) //There is an object with name key and it is a CJsonValueArray
	{

		const CJsonValue *tmp = (const CJsonValue*)json_object[key];
		CJsonArray *array = new CJsonArray((CJsonArray*)tmp);

		RMessageArray * result = new RMessageArray;
		result->setArray(*array);

		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}


RMessageArray& RMessageBody::getArray(std::string key){
	if (json_object[key] && json_object[key]->GetType() == 3) //There is an object with name key and it is a CJsonValueArray
	{

		const CJsonValue *tmp = (const CJsonValue*)json_object[key];
		CJsonArray *array = new CJsonArray((CJsonArray*)tmp);

		RMessageArray * result = new RMessageArray;
		result->setArray(*array);

		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}


RMessageBody& RMessageBody::getInnerBody(std::string key){
	if (json_object[key] && json_object[key]->GetType() == 2) //There is an object with name key and it is a CJsonValueObject
	{

		const CJsonValue *tmp = (const CJsonValue*)json_object[key];
		CJsonObject *object = new CJsonObject((CJsonObject*)tmp);

		RMessageBody * result = new RMessageBody;
		result->setObject(*object);

		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}

RMessageBody& RMessageBody::getInnerBody(char* key){
	if (json_object[key] && json_object[key]->GetType() == 2) //There is an object with name key and it is a CJsonValueObject
	{

		const CJsonValue *tmp = (const CJsonValue*)json_object[key];
		CJsonObject *object = new CJsonObject((CJsonObject*)tmp);

		RMessageBody * result = new RMessageBody;
		result->setObject(*object);

		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}

void RMessageBody::setObject(CJsonObject& object){
	json_object = object;
}
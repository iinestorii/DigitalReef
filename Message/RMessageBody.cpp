#include "RMessageBody.h"
#include "RMessageArray.h"
#include <iostream>
#include <cjsonvalue_common.h>

RMessageBody::RMessageBody(void){
}


RMessageBody::~RMessageBody(void){
	}


int RMessageBody::addSimplex(std::string key, char * value){
	std::string valueStr = value; //json detects chars as bools if not handled
	return addSimplex(key, valueStr);
}

int RMessageBody::addSimplex(std::string key, int value){
	if (value < 0)return -1; //json-plus can't handle negative integers
	int oldValueSize = getPredLength(key); // 0 if no entry found with key 'key', length of value stored under key otherwise
	int futureSize;
	if (oldValueSize){
		futureSize = CUR_SIZE - oldValueSize + std::to_string(value).size();
	}
	else{
		futureSize = CUR_SIZE + key.size() + std::to_string(value).size() + 4;
	}
	if (futureSize < MAX_SIZE){
		json_object.AddPare(key, (jint)value);
		CUR_SIZE = futureSize;
		return 1;
	}
	return -1;
}

int RMessageBody::addSimplex(std::string key, std::string value){
	int oldValueSize = getPredLength(key);
	int futureSize;
	if (oldValueSize){
		futureSize = CUR_SIZE - oldValueSize + value.size() + 2;
	}
	else{
		futureSize = CUR_SIZE + key.size() + value.size() + 6;
	}
	if (futureSize < MAX_SIZE){
		json_object.AddPare(key, value);
		CUR_SIZE = futureSize;
		return 1;
	}
	return -1;
}

int RMessageBody::addSimplex(std::string key, bool value){
	int valueSize=0;
	if (value) valueSize = 4; 
	else valueSize = 5;

	int oldValueSize = getPredLength(key);
	int futureSize;
	if (oldValueSize){
		futureSize = CUR_SIZE - oldValueSize + valueSize;
	}
	else{
		futureSize = CUR_SIZE + key.size() + valueSize + 4;
	}

	if (futureSize < MAX_SIZE){
		json_object.AddPare(key, value);
		CUR_SIZE = futureSize;
		return 1;
	}
	return -1;
}

int RMessageBody::addInnerBody(std::string key, RMessageBody  &innerbody){
	CJsonObject  &innerObject = innerbody.getObject();
	int oldValueSize = getPredLength(key);
	int futureSize;
	if (oldValueSize){
		futureSize = CUR_SIZE - oldValueSize + innerbody.getSize();
	}
	else{
		futureSize = CUR_SIZE + key.size() + innerbody.getSize() + 4;
	}

	if (futureSize < MAX_SIZE){
		json_object.AddPare(key, &innerObject);
		CUR_SIZE = futureSize;
		return -1;
	}
	return 1;
	
	
}

int RMessageBody::getPredLength(std::string key){

	std::vector <jstring> names;
	json_object.GetNames(names);
	int result = 0;

	for (std::vector<jstring>::iterator it = names.begin(); it != names.end(); ++it) {
		if (*it == key){			
			int switchVar = json_object[key]->GetType();			
			switch (json_object[key]->GetType()){
				case 0: //string					
					result = caseString(key);
					break;
				case 1: //int
					result = caseInt(key);
					break;
				case 2: //object
					result = caseObject(key);
					break;
				case 3://array
					result = caseArray(key);
					break;
				case 5: //bool				
					result = caseBool(key);
					break;
			}
			break;
		}
	}
	return result;
}

int RMessageBody::caseString(std::string key){
	std::string value = getString(key);
	return value.size();
}

int RMessageBody::caseInt(std::string key){
	int value = getInt(key);
	return std::to_string(value).size();
}

int RMessageBody::caseObject(std::string key){
	const CJsonValue *tmp = (const CJsonValue*)json_object[key];
	CJsonObject *value = new CJsonObject((CJsonObject*)tmp);
	int result = value->ToString().size();
	delete value;
	delete tmp;	
	return result;
}

int RMessageBody::caseArray(std::string key){
	const CJsonValue *tmp = (const CJsonValue*)json_object[key];
	CJsonArray *value = new CJsonArray((CJsonArray*)tmp);
	int result = value->ToString().size();
	delete value;
	delete tmp;
	return result;
}

int RMessageBody::caseBool(std::string key){
	bool value = getBool(key);
	int result;
	if (value) result = 4;
	else result = 5;
	return result;
}

int RMessageBody::addArray(std::string key, RMessageArray& value){
	int oldValueSize = getPredLength(key);
	int futureSize;
	if (oldValueSize){
		futureSize = CUR_SIZE - oldValueSize + value.getSize();
	}
	else{
		futureSize = CUR_SIZE + key.size() + value.getSize() + 4;
	}

	if (futureSize < MAX_SIZE){
		json_object.AddPare(key, &value.getArray());
		CUR_SIZE = futureSize;
		return 1;
	}
	return -1;
	
}

int RMessageBody::getSize(){
	return CUR_SIZE;
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

void RMessageBody::setJsonObject(std::string json){
	json_object = *CJsonParser::Execute((jstring)json);
	CUR_SIZE = json.size();
}


std::string RMessageBody::toString(){
		return json_object.ToString();
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
		delete array;
		delete tmp;
		
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
		delete array;
		delete tmp;

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
		delete object;
		delete tmp;
	
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
		delete object;
		delete tmp;
		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}

void RMessageBody::setObject(CJsonObject& object){
	json_object = object;
}

void RMessageBody::clear(){
	json_object.Clear();
	CUR_SIZE = 1;
}
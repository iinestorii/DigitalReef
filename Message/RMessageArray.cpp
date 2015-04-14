#include "RMessageArray.h"
#include <iostream>


RMessageArray::RMessageArray(void)
{
}


RMessageArray::~RMessageArray(void)
{
}


void RMessageArray::addSimplex(char * value){
	json_array.AddMember(value);
}

void RMessageArray::addSimplex(int value){
	json_array.AddMember((jint)value);
}

void RMessageArray::addSimplex(std::string value){

	json_array.AddMember(value);
}

void RMessageArray::addSimplex(bool value){
	json_array.AddMember(value);
}

void RMessageArray::addInnerBody(RMessageBody  &innerbody){
	CJsonObject  &innerObject = innerbody.getObject();
	json_array.AddMember(&innerObject);

}

void RMessageArray::addArray(std::string key){
	CJsonArray innerArray;
	json_array.AddMember(&innerArray);
}

CJsonArray RMessageArray::getArray(){
	return &json_array;
}

void RMessageArray::setArray(CJsonArray& array){
	json_array = array;
}

std::string RMessageArray::toString(){
	return json_array.ToString();
}



std::string RMessageArray::getString(int key){

	if (json_array[key] && json_array[key]->GetType() == 0) //There is an object with name key and it is a CJsonValueNumber
	{
		const CJsonValue* cvn = json_array[key];
		std::string result;
		result = cvn->ToString();
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}



int RMessageArray::getInt(int key){

	if (json_array[key] && json_array[key]->GetType() == 1) //There is an object with name key and it is a CJsonValueNumber
	{
		// dynamic_cast used as CJsonValueNumber*->getValue is not 
		// a virtual method and can't be reached with 
		// CJsonValueNumber, alternatively there could have been made some changes
		// in the jsonplus.lib
		const CJsonValueNumber* cvn = dynamic_cast<const CJsonValueNumber*>(json_array[key]);
		int result;
		cvn->GetValue(result);
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}

bool RMessageArray::getBool(int key){

	if (json_array[key] && json_array[key]->GetType() == 5) //There is an object with name key and it is a CJsonValueNumber
	{
		const CJsonValue* cvn = json_array[key];
		bool result;
		std::istringstream(cvn->ToString()) >> std::boolalpha >> result;
		result;
		return result;
	}
	throw CJsonError(INVALID_PARAM);
}


RMessageArray& RMessageArray::getArray(int key){
	if (json_array[key] && json_array[key]->GetType() == 3) //There is an object with name key and it is a CJsonValueArray
	{

		const CJsonValue *tmp = (const CJsonValue*)json_array[key];
		CJsonArray *array = new CJsonArray((CJsonArray*)tmp);

		RMessageArray * result = new RMessageArray;
		result->setArray(*array);

		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}



RMessageBody& RMessageArray::getInnerBody(int key){
	if (json_array[key] && json_array[key]->GetType() == 2) //There is an object with name key and it is a CJsonValueObject
	{

		const CJsonValue *tmp = (const CJsonValue*)json_array[key];
		CJsonObject *object = new CJsonObject((CJsonObject*)tmp);

		RMessageBody * result = new RMessageBody;
		result->setObject(*object);

		return *result;
	}
	throw CJsonError(INVALID_PARAM);
}


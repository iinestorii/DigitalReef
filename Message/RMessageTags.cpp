#include "RMessageTags.h"
#include <cjson.h>
#include <iostream>

RMessageTags::RMessageTags()
{
}


RMessageTags::~RMessageTags()
{
 }

RMessageTags::RMessageTags(const RMessageTags * tags){
	json_array = (CJsonArray&)tags->json_array;
	CUR_SIZE = json_array.ToString().size();	
}

int RMessageTags::addTag(std::string tag){
	std::string sys_msg = "SYS_newMember";
	if (tag == sys_msg)return -1;
	int futureSize = CUR_SIZE + tag.size() + 3;
	if (futureSize <= MAX_SIZE){
		CUR_SIZE = futureSize;
		json_array.AddMember(tag);
		return 1;
	}
	else{
		return -1;
	}
	
}

int RMessageTags::addTag(char* tag){
	return addTag((std::string)tag);
}

/*
void RMessageTags::initiateTagsWithArray(CJsonArray& jsonArray){

	for (unsigned i = 0; i < jsonArray.Size(); i++)
	{
		json_array.AddMember(jsonArray[i]);
	}

}
*/

void RMessageTags::setArray(CJsonArray& array){
	json_array = array;
	CUR_SIZE = array.ToString().size();
}


bool RMessageTags::containsAnyOf(char* input_tag){
	
	std::string temp1;	
	std::string temp2 = "\"" + (std::string)input_tag + "\"";
	
	for (unsigned i = 0; i < json_array.Size(); i++)
	{
		temp1 = json_array[i]->ToString();
			if ( temp1 == temp2){
				return true;
			}		
	}
	return false;
}

bool RMessageTags::containsAnyOf(std::string input_tag){
	std::string temp1;
	std::string temp2 = "\"" + input_tag + "\"";
	for (unsigned i = 0; i < json_array.Size(); i++)
	{
		temp1 = json_array[i]->ToString();
		if (temp1 == temp2){
			return true;
		}
	}
	return false;
}


bool RMessageTags::containsAnyOf(std::vector<std::string> input_tags){
	std::string temp1;
	std::string temp2;
	for (std::vector<std::string>::iterator it = input_tags.begin(); it != input_tags.end(); ++it) {
		temp2 = "\"" + *it + "\"";
		for (unsigned i = 0; i < json_array.Size(); i++)
		{
			temp1 = json_array[i]->ToString();
			if (temp1 == temp2){				
				return true;
			}
		}		
	}
	return false;
}

std::string RMessageTags::getTags(){
	return json_array.ToString();
}

void RMessageTags::clear(){
	json_array.~CJsonArray();
	new (&json_array) CJsonArray();
	CUR_SIZE = 1;
}
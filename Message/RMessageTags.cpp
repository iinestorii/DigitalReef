#include "RMessageTags.h"
#include <cjson.h>
#include <iostream>

RMessageTags::RMessageTags()
{
}


RMessageTags::~RMessageTags()
{
}

void RMessageTags::addTag(std::string tag){
	json_array.AddMember(tag);
}

void RMessageTags::addTag(char* tag){
	json_array.AddMember(tag);
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
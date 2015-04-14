//==================================================
//include guard
//==================================================
#ifndef __RMessageTags_H_INCLUDED__
#define __RMessageTags_H_INCLUDED__

//==================================================
// include dependencies
//==================================================
#include <cjson.h>
//==================================================
// class declaration
//==================================================
class RMessageTags
{
public:
	void addTag(std::string);
	void addTag(char*);

	bool containsAnyOf(char*);
	bool containsAnyOf(std::string);
	bool containsAnyOf(std::vector<std::string>);

	RMessageTags();
	~RMessageTags();

private:
	CJsonArray json_array;
};
#endif



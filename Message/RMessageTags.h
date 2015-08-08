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
	friend class RMessage;
public:
	int addTag(std::string);
	int addTag(char*);

	bool containsAnyOf(char*);
	bool containsAnyOf(std::string);
	bool containsAnyOf(std::vector<std::string>);

	RMessageTags();
	~RMessageTags();

protected:
	void initiateTagsWithArray(CJsonArray&); //should be protected
	void setArray(CJsonArray&);
	std::string getTags();

	RMessageTags(const RMessageTags * value);

	void clear();
private:
	int CUR_SIZE=1;
	int MAX_SIZE = 70;
	CJsonArray json_array;
};
#endif



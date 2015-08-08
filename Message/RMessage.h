//==================================================
//include guard
//==================================================
#ifndef __RMessage_H_INCLUDED__
#define __RMessage_H_INCLUDED__

//==================================================
// include dependencies
//==================================================
#include <RMessageBody.h>
#include <RMessageArray.h>
#include <RMessageTags.h>
//==================================================
// class declaration
//==================================================
class RMessage{
	friend class Reef;
	friend class ReefSatellite;
public:
	int addSimplex(std::string, int);
	int addSimplex(std::string, std::string);
	int addSimplex(std::string, bool);
	int addSimplex(std::string, char*);
	int addArray(std::string, RMessageArray&);
	int addInnerBody(std::string, RMessageBody&);

	int addTag(char*);
	int addTag(std::string);
	//should be protected


	std::string getString(std::string);
	std::string getString(char*);
	int getInt(std::string);
	int getInt(char*);
	bool getBool(std::string);
	bool getBool(char*);
	RMessageArray& getArray(std::string);
	RMessageArray& getArray(char*);
	RMessageBody& getInnerBody(std::string);
	RMessageBody& getInnerBody(char*);
	std::string getTags();
	std::string getBody();

	bool containsAnyOf(char*);
	bool containsAnyOf(std::string);
	bool containsAnyOf(std::vector<std::string>);

	RMessage();
	~RMessage();
	RMessage(const RMessage * value);

protected:
	void initiateTagsWithArray(CJsonArray*);
	void clear();
	void initiateWithJson(std::string);

private:
	RMessageBody body;
	RMessageTags tags;

};
#endif


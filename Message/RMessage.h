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
public:
	void addSimplex(std::string, int);
	void addSimplex(std::string, std::string);
	void addSimplex(std::string, bool);
	void addSimplex(std::string, char*);
	void addArray(std::string, RMessageArray&);
	void addInnerBody(std::string, RMessageBody&);

	void addTag(char*);
	void addTag(std::string);
	bool containsAnyOf(char*);
	bool containsAnyOf(std::string);
	bool containsAnyOf(std::vector<std::string>);

	void initiateWithJson(std::string);

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

	std::string getBody();
	RMessage();
	~RMessage();


private:
	RMessageBody body;
	RMessageTags tags;

};
#endif


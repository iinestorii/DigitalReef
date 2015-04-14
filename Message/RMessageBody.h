//==================================================
//include guard
//==================================================
#ifndef __RMessageBody_H_INCLUDED__
#define __RMessageBody_H_INCLUDED__

//==================================================
// include dependencies
//==================================================
#include <string>


#if defined __unix__
#include <jsonplus/cjson.h>
#include <jsonplus/cjsonparser.h>
#elif defined _WIN32
#include <cjson.h>
#endif

//==================================================
// class declaration
//==================================================
class RMessageArray;
class RMessageBody{
public:
	void addSimplex(std::string, int);
	void addSimplex(std::string, std::string);
	void addSimplex(std::string, bool);
	void addSimplex(std::string, char*);
	void addInnerBody(std::string, RMessageBody&);
	void addArray(std::string, RMessageArray&);

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

	void setJsonObject(std::string);
	void initiateWithJson(std::string);
	CJsonObject getObject();
	void setObject(CJsonObject&);
	std::string toString();
	RMessageBody();
	~RMessageBody();
private:
	CJsonObject json_object;


};
#endif



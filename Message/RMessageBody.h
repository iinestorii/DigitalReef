//==================================================
//include guard
//==================================================
#ifndef __RMessageBody_H_INCLUDED__
#define __RMessageBody_H_INCLUDED__

//==================================================
// include dependencies
//==================================================
#include <string>
#include <memory>


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
	friend class RMessage;
	friend class RMessageArray;
public:
	int addSimplex(std::string, int);
	int addSimplex(std::string, std::string);
	int addSimplex(std::string, bool);
	int addSimplex(std::string, char*);
	int addInnerBody(std::string, RMessageBody&);
	int addArray(std::string, RMessageArray&);

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
	std::string toString();
	RMessageBody();
	~RMessageBody();

protected:
	void setJsonObject(std::string);
	void initiateWithJson(std::string);
	CJsonObject getObject();
	void setObject(CJsonObject&);


	int getPredLength(std::string);
	int getSize();
	void clear();
private:
	CJsonObject json_object;


	int CUR_SIZE = 1;
	int MAX_SIZE = 4930;

	int caseString(std::string);
	int caseBool(std::string);
	int caseInt(std::string);
	int caseObject(std::string);
	int caseArray(std::string);
};
#endif
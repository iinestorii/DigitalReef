//==================================================
//include guard
//==================================================
#ifndef __RMessageArray_H_INCLUDED__
#define __RMessageArray_H_INCLUDED__

//==================================================
// include dependencies
//==================================================
#include <string>
#include <RMessageBody.h>

#if defined __unix__
#include <jsonplus/cjson.h>
#include <jsonplus/cjsonparser.h>
#elif defined _WIN32
#include <cjson.h>
#endif

//==================================================
// class declaration
//==================================================
class RMessageArray{
	friend class RMessageBody;
public:
	void addSimplex(int);
	void addSimplex(std::string);
	void addSimplex(bool);
	void addInnerBody(RMessageBody&);
	void addArray(RMessageArray&);
	CJsonArray getArray();
	
	std::string toString();
	RMessageArray();
	~RMessageArray();

	std::string getString(int);
	int getInt(int);
	bool getBool(int);
	RMessageArray& getArray(int);
	RMessageBody& getInnerBody(int);
	int getSize();

protected:
	void setArray(CJsonArray&);
private:
	CJsonArray json_array;
	int CUR_SIZE = 1;


};
#endif





#include <RMessage.h>
#include "RMessageArray.h"
#include <iostream>
#include <string>

int main(int argc, char *argv[]){
	RMessage msg;
	msg.addSimplex("integer_1", 333);
	msg.addSimplex("integer_2", 7);
	msg.addSimplex("char_1", "test");
	msg.addSimplex("char_2", "test");
	msg.addSimplex("string_1", (std::string)"test");
	msg.addSimplex("string_2", (std::string)"test");
	msg.addSimplex("bool_1", false);
	msg.addSimplex("bool_2", true);

	RMessageBody body;
	body.addSimplex("integer_3", 5);
	body.addSimplex("integer_4", 7);
	body.addSimplex("char_3", "test");
	body.addSimplex("char_4", "test");
	body.addSimplex("string_3", (std::string)"test");
	body.addSimplex("string_4", (std::string)"test");
	body.addSimplex("bool_3", false);
	body.addSimplex("bool_4", true);

	RMessageArray array;
	array.addSimplex(5);
	array.addSimplex(7);
	array.addSimplex("test");
	array.addSimplex("test");
	array.addSimplex((std::string)"test");
	array.addSimplex((std::string)"test");
	array.addSimplex(false);
	array.addSimplex(true);

	//msg.addArray("array", array);

	RMessageArray array2;
	array2.addSimplex(5);
	array2.addSimplex(7);
	array2.addSimplex("test");
	array2.addSimplex("test");
	array2.addSimplex((std::string)"test");
	array2.addSimplex((std::string)"test");
	array2.addSimplex(false);
	array2.addSimplex(true);

	RMessageBody body2;
	body2.addSimplex("integer_5", 5);
	body2.addSimplex("integer_6", 7);

	body.addArray("array_in_body", array2);
	body.addInnerBody("body2", body2);

	RMessageArray array3;
	array3.addSimplex(5);
	array3.addInnerBody(body);

	msg.addArray("array3", array3);
	msg.addInnerBody("innerbody", body);
	std::string builder = msg.getBody();
	RMessage msg2;
	msg2.initiateWithJson(builder);
	msg2.addSimplex("teststring", "test_string");
	msg2.addSimplex("testinteger", 13);
	msg2.addSimplex("testfalse", false);
	msg2.addSimplex("testtrue", true);

	RMessageArray array4;
	array4.addSimplex("string in array4");
	array4.addSimplex(4);
	array4.addSimplex(true);
	RMessageBody body3;
	body3.addSimplex("body3-Int",3);
	array4.addInnerBody(body3);
	msg2.addArray("array4 in msg2", array4);

	RMessageBody body4;
	body4.addSimplex("Body4-String", "String in body4");
	body4.addSimplex("Body4-Int", 5);
	body4.addSimplex("Body4-Bool", false);
	msg2.addInnerBody("body4", body4);

	std::cout << "msg2 TestString:: " << msg2.getString("teststring") << std::endl;
	std::cout << "msg2 TestInt:: " << msg2.getInt("testinteger") << std::endl;
	bool testfalse = msg2.getBool("testfalse");
	std::cout << "msg2 TestBool false:: " << testfalse << std::endl;
	std::cout << "msg2 TestBool true:: " << msg2.getBool("testtrue") << std::endl;

	RMessageArray array4Extract = msg2.getArray("array4 in msg2");
	std::cout << "msg2 TestArray4:: "  << std::endl;
	std::cout <<  array4Extract.toString() << std::endl;
	std::cout << array4Extract.getString(0) << std::endl;
	std::cout << array4Extract.getInnerBody(3).toString() << std::endl;

	RMessageBody body4Extract = msg2.getInnerBody("body4");
	std::cout << "msg2 Body4 extracted:: " << std::endl;
	std::cout << body4Extract.toString() << std::endl;
	
	msg2.addTag("testtag");
	msg2.addTag((std::string)"testtag2");
	msg2.addTag("testtag3");
	msg2.addTag("testtag4");
	std::vector<std::string> string_vector_true;
	std::vector<std::string> string_vector_false;
	string_vector_true.push_back("others1");
	string_vector_true.push_back("testtag3");
	string_vector_true.push_back("others2");
	string_vector_false.push_back("others1");
	string_vector_false.push_back("testtagfalse");
	string_vector_false.push_back("others2");
	std::cout << "False_Tags: " << msg2.containsAnyOf(string_vector_false) << std::endl;
	std::cout << "True_Tags: " << msg2.containsAnyOf(string_vector_true) << std::endl;
	
	return 0;
}
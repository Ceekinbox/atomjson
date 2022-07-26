#include "pch.h"
#include "CppUnitTest.h"
#include "..\..\vs\atomjson\atomjson.h"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define UNIT_TEST_STRING(expect,json)\
	do{\
		atom::CJsonValue v;\
		v._init();\
		Assert::AreEqual(int(atom::PARSE_OK),atom::parse(&v,json));\
		Assert::AreEqual(int(atom::ATOM_STRING), int(v.get_type()));\
		Assert::AreEqual(expect,v.get_string());\
		Assert::AreEqual(sizeof(expect) - 1,v.get_string_length());\
		v._free();\
	}while(0)

void inline UNIT_TEST_ERROR(int error,const char* json) {
	atom::CJsonValue v; 
	v._init(); 
	Assert::AreEqual(error, atom::parse(&v, json)); 
	Assert::AreEqual(int(atom::ATOM_NULL), int(v.get_type())); 
	v._free();
}
void inline UNIT_TEST_BOOLEN(bool expect,const char* json) {
	atom::CJsonValue v; 
	v._init(); 
	Assert::AreEqual(int(atom::PARSE_OK), atom::parse(&v, json));
	if(expect)
		Assert::AreEqual(int(atom::ATOM_TRUE), int(v.get_type()));
	else
		Assert::AreEqual(int(atom::ATOM_FALSE), int(v.get_type()));
	Assert::AreEqual(int(expect), int(v.get_boolen()));
	v._free();
}
void inline UNIT_TEST_NULL(const char* json) {
	atom::CJsonValue v;
	v._init();
	Assert::AreEqual(int(atom::PARSE_OK), atom::parse(&v, json));
	Assert::AreEqual(int(atom::ATOM_NULL), int(v.get_type()));
	v._free();
}

void inline UNIT_TEST_NUMBER(double expect, const char* json) {
	atom::CJsonValue v;
	v._init();
	Assert::AreEqual(int(atom::PARSE_OK), atom::parse(&v, json));
	Assert::AreEqual(int(atom::ATOM_NUMBER), int(v.get_type()));
	Assert::AreEqual(expect, v.get_number());
	v._free();
}

namespace UnitTest1
{
	TEST_CLASS(UnitTestParse)
	{
	public:
		TEST_METHOD(test_parse_string_0) { UNIT_TEST_STRING("\x24", "\"\\u0024\"");}
		TEST_METHOD(test_parse_string_1) { UNIT_TEST_STRING("\xC2\xA2", "\"\\u00A2\""); }
		TEST_METHOD(test_parse_string_2) { UNIT_TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); }
		TEST_METHOD(test_parse_string_3) { UNIT_TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\""); }
		TEST_METHOD(test_parse_string_4) { UNIT_TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\""); }
	};

	TEST_CLASS(UnitTestError)
	{
	public:
		TEST_METHOD(test_parse_invalid_unicode_0) { UNIT_TEST_ERROR(atom::PARSE_INVALID_UNICODE_HEX, "\"\\u 123\""); }
		TEST_METHOD(test_parse_invalid_unicode_1) { UNIT_TEST_ERROR(atom::PARSE_INVALID_UNICODE_HEX, "\"\\uD834\\u 123\""); }
		TEST_METHOD(test_parse_invalid_unicode_2) { UNIT_TEST_ERROR(atom::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD834x\""); }
		TEST_METHOD(test_parse_invalid_unicode_3) { UNIT_TEST_ERROR(atom::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD834\\x\""); }
		TEST_METHOD(test_parse_invalid_unicode_4) { UNIT_TEST_ERROR(atom::PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD834\\u0001\""); }
	};
	TEST_CLASS(UnitTestArray)
	{
	public:
		TEST_METHOD(test_parse_array_0) {
			atom::CJsonValue v;
			v._init();
			Assert::AreEqual(int(atom::PARSE_OK), atom::parse(&v, "[ ]"));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_type()));
			v._free();
		}
		TEST_METHOD(test_parse_array_1) {
			atom::CJsonValue v;
			v._init();
			Assert::AreEqual(int(atom::PARSE_OK), atom::parse(&v, "[ null , false , true , 123 , \"abc\" ]"));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_type()));

			Assert::AreEqual(5, int(v.get_array_size()));
			Assert::AreEqual(int(atom::ATOM_NULL), int(v.get_array_element(0).get_type()));
			Assert::AreEqual(int(atom::ATOM_FALSE), int(v.get_array_element(1).get_type()));
			Assert::AreEqual(int(atom::ATOM_TRUE), int(v.get_array_element(2).get_type()));
			Assert::AreEqual(int(atom::ATOM_NUMBER), int(v.get_array_element(3).get_type()));
			Assert::AreEqual(int(atom::ATOM_STRING), int(v.get_array_element(4).get_type()));
			Assert::AreEqual(123.0, v.get_array_element(3).get_number());
			Assert::AreEqual("abc", v.get_array_element(4).get_string());
			v._free();
		}
		TEST_METHOD(test_parse_array_2) {
			atom::CJsonValue v;
			v._init();
			Assert::AreEqual(int(atom::PARSE_OK), atom::parse(&v, "[ [ ] , [ 0 ] , [ 0 , 1 ] , [ 0 , 1 , 2 ] ]"));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_type()));

			Assert::AreEqual(4, int(v.get_array_size()));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_array_element(0).get_type()));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_array_element(1).get_type()));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_array_element(2).get_type()));
			Assert::AreEqual(int(atom::ATOM_ARRAY), int(v.get_array_element(3).get_type()));

			Assert::AreEqual(int(atom::ATOM_NUMBER), int(v.get_array_element(1).get_array_element(0).get_type()));

			v._free();
		}
	};
#if 1
	TEST_CLASS(UnitTest___)
	{
	public:
		TEST_METHOD(test_0) {
			UNIT_TEST_BOOLEN(true, "true");
		}
		TEST_METHOD(test_1) {
			UNIT_TEST_BOOLEN(false, "false");
		}
		TEST_METHOD(test_2) {
			UNIT_TEST_NULL("null");
		}
		TEST_METHOD(test_3) {
			UNIT_TEST_NUMBER(1.0, "1.0");
			UNIT_TEST_NUMBER(-1.0, "-1.0");
			UNIT_TEST_NUMBER(1.23, "1.23");
		}
	};
#endif

}


#include <iostream>
#include <cstdlib>
#include <string>

#include "atomjson.h"
using namespace atom;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;



#define EXPECT_EQ_BASE(equality,expect,actual,format)\
	do {\
		test_count++;\
		if (equality)\
			test_pass++;\
		else {\
			fprintf(stderr, "%s:%d: expect:" format "actual:" format "\n", __FILE__, __LINE__, expect, actual);\
			main_ret = 1;}\
	}while (0);

#define EXPECT_EQ_INT(expect,actual)do{EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d");}while(0);

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define EXPECT_EQ_STRING(expect, actual, alength) \
    EXPECT_EQ_BASE(sizeof(expect) - 1 == (alength) && memcmp(expect, actual, alength) == 0, expect, actual, "%s")
#define EXPECT_EQ_TRUE(actual) EXPECT_EQ_BASE((actual) == true,true,actual,"%d")
#define EXPECT_EQ_FALSE(actual) EXPECT_EQ_BASE((actual) == false,false,actual,"%d")

#define TEST_ERROR(error, json)\
    do {\
        CJsonValue v;\
        v.type = ATOM_FALSE;\
        EXPECT_EQ_INT(error, parse(&v, json));\
        EXPECT_EQ_INT(ATOM_NULL, v.get_type());\
    } while(0)

#define TEST_NUMBER(expect,json)\
	do{\
		CJsonValue v;\
		EXPECT_EQ_INT(PARSE_OK,parse(&v,json));\
		EXPECT_EQ_INT(ATOM_NUMBER,v.get_type());\
		EXPECT_EQ_DOUBLE(expect,v.get_number());\
	}while(0)

#define TEST_STRING(expect,json)\
	do{\
		CJsonValue v;\
		v._init();\
		EXPECT_EQ_INT(PARSE_OK,parse(&v,json));\
		EXPECT_EQ_INT(ATOM_STRING, v.get_type());\
		EXPECT_EQ_STRING(expect,v.get_string(),v.get_string_length());\
		v._free();\
	} while(0)
static void test_parse_null() {
	CJsonValue v;
	v.type = ATOM_TRUE;
	EXPECT_EQ_INT(ATOM_TRUE, v.get_type());
	EXPECT_EQ_INT(PARSE_OK, parse(&v, "null"));
}

static void test_parse_true() {
	CJsonValue v;
	v.type = ATOM_TRUE;
	EXPECT_EQ_INT(ATOM_TRUE, v.get_type());
	EXPECT_EQ_INT(PARSE_OK, parse(&v, "true"));
}

static void test_parse_false() {
	CJsonValue v;
	v.type = ATOM_FALSE;
	EXPECT_EQ_INT(ATOM_FALSE, v.get_type());
	EXPECT_EQ_INT(PARSE_OK, parse(&v, "false"));
}

static void test_parse_number() {
	TEST_NUMBER(0.0, "0");
	TEST_NUMBER(0.0, "-0");
	TEST_NUMBER(0.0, "-0.0");
	TEST_NUMBER(1.0, "1");
	TEST_NUMBER(-1.0, "-1");
	TEST_NUMBER(1.5, "1.5");                                                                                                                                                                                                                                                                                                                                                                                                                                        
	TEST_NUMBER(-1.5, "-1.5");
	TEST_NUMBER(3.1416, "3.1416");
	TEST_NUMBER(1E10, "1E10");
	TEST_NUMBER(1e10, "1e10");
	TEST_NUMBER(1E+10, "1E+10");
	TEST_NUMBER(1E-10, "1E-10");
	TEST_NUMBER(-1E10, "-1E10");
	TEST_NUMBER(-1e10, "-1e10");
	TEST_NUMBER(-1E+10, "-1E+10");
	TEST_NUMBER(-1E-10, "-1E-10");
	TEST_NUMBER(1.234E+10, "1.234E+10");
	TEST_NUMBER(1.234E-10, "1.234E-10");
	TEST_NUMBER(0.0, "1e-10000"); /* must underflow */
}

static void test_parse_invalid_value() {
	TEST_ERROR(PARSE_INVALID_VALUE, "+0");
	TEST_ERROR(PARSE_INVALID_VALUE, "+1");
	TEST_ERROR(PARSE_INVALID_VALUE, ".123"); /* at least one digit before '.' */
	TEST_ERROR(PARSE_INVALID_VALUE, "1.");   /* at least one digit after '.' */
	TEST_ERROR(PARSE_INVALID_VALUE, "INF");
	TEST_ERROR(PARSE_INVALID_VALUE, "inf");
	TEST_ERROR(PARSE_INVALID_VALUE, "NAN");
	TEST_ERROR(PARSE_INVALID_VALUE, "nan");
}

static void test_parse_string() {
#if 0
	TEST_STRING("", "\"\"");
	TEST_STRING("Hello", "\"Hello\"");
	TEST_STRING("a","\"a\"");
	TEST_STRING("Hello\nWorld", "\"Hello\\nWorld\"");
	TEST_STRING("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
#endif
#if 1
	TEST_STRING("\x24", "\"\\u0024\"");         /* Dollar sign U+0024 */
	TEST_STRING("\xC2\xA2", "\"\\u00A2\"");     /* Cents sign U+00A2 */
	TEST_STRING("\xE2\x82\xAC", "\"\\u20AC\""); /* Euro sign U+20AC */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\uD834\\uDD1E\"");  /* G clef sign U+1D11E */
	TEST_STRING("\xF0\x9D\x84\x9E", "\"\\ud834\\udd1e\"");  /* G clef sign U+1D11E */
#endif
}
static void test_invalid_value() {
	TEST_ERROR(PARSE_INVALID_VALUE, "abc");
	TEST_ERROR(PARSE_INVALID_VALUE, " abc ");
	TEST_ERROR(PARSE_INVALID_VALUE, "abc ");
	TEST_ERROR(PARSE_INVALID_VALUE, "true ");
}
static void test_parse_invalid_string_escape() {
	TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\v\"");
	TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\'\"");
	TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\0\"");
	TEST_ERROR(PARSE_INVALID_STRING_ESCAPE, "\"\\x12\"");
}

static void test_parse_invalid_string_char() {
	TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x01\"");
	TEST_ERROR(PARSE_INVALID_STRING_CHAR, "\"\x1F\"");
}
static void test_parse_invalid_unicode_hex() {
	TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
	TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u0123\"");
}

static void test_parse_invalid_unicode() {
	TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\u 123\"");
	TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD834x\"");
	TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD834\\x\"");
	TEST_ERROR(PARSE_INVALID_UNICODE_HEX, "\"\\uD834\\u 123\"");
	TEST_ERROR(PARSE_INVALID_UNICODE_SURROGATE, "\"\\uD834\\u0001\"");

}

static void test_access_string() {
	CJsonValue v;
	v._init();
	v.set_string("", 0);
	EXPECT_EQ_STRING("", v.get_string(), v.get_string_length());
	v.set_string("Hello", 5);
	EXPECT_EQ_STRING("Hello", v.get_string(), v.get_string_length());
	v._free();
}

static void test_access_boolen_true() {
	CJsonValue v;
	v._init();
	v.set_boolen(true);
	EXPECT_EQ_TRUE(v.get_boolen());
	v._free();
}

static void test_access_boolen_false() {
	CJsonValue v;
	v._init();
	v.set_boolen(false);
	EXPECT_EQ_FALSE(v.get_boolen());
	v._free();
}

static void test_access_number() {
	CJsonValue v;
	v._init();
	v.set_string("a", 1);
	v.set_number(1234.5);
	EXPECT_EQ_DOUBLE(1234.5, v.get_number());
	v._free();
}

static void test_parse() {
	//test_parse_null();
	//test_parse_true();
	//test_parse_false();
	//test_access_string();
	//test_access_boolen_false();
	//test_access_boolen_true();
	//test_access_number();
	test_parse_string();
	//test_parse_invalid_string_escape();
	//test_parse_invalid_string_char();

	//test_error();
	//test_parse_number();
}

static void test_error() {
	//test_parse_invalid_unicode();
}
int main() {
#if 1
	test_parse();
	test_error();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
#endif
}
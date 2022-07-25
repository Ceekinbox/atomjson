
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

#define EXPECT_EQ_INT(expect,actual)do{EXPECT_EQ_BASE(expect == actual, expect, actual, "%d");}while(0);

#define EXPECT_EQ_DOUBLE(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%.17g")

#define TEST_ERROR(error, json)\
    do {\
        CJsonValue v;\
        v.type = ATOM_FALSE;\
        EXPECT_EQ_INT(error, parse(&v, json));\
        EXPECT_EQ_INT(ATOM_NULL, get_type(&v));\
    } while(0)

#define TEST_NUMBER(expect,json)\
	do{\
		CJsonValue v;\
		EXPECT_EQ_INT(PARSE_OK,parse(&v,json));\
		EXPECT_EQ_INT(ATOM_NUMBER,get_type(&v));\
		EXPECT_EQ_DOUBLE(expect,get_number(&v));\
	}while(0)

static void test_parse_null() {
	CJsonValue v;
	v.type = ATOM_TRUE;
	EXPECT_EQ_INT(ATOM_TRUE, get_type(&v));
	EXPECT_EQ_INT(PARSE_OK, parse(&v, "null"));
}

static void test_parse_true() {
	CJsonValue v;
	v.type = ATOM_TRUE;
	EXPECT_EQ_INT(ATOM_TRUE, get_type(&v));
	EXPECT_EQ_INT(PARSE_OK, parse(&v, "true"));
}

static void test_parse_false() {
	CJsonValue v;
	v.type = ATOM_FALSE;
	EXPECT_EQ_INT(ATOM_FALSE, get_type(&v));
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

static void test_error() {
	TEST_ERROR(PARSE_INVALID_VALUE, "abc");
	TEST_ERROR(PARSE_INVALID_VALUE, " abc ");
	TEST_ERROR(PARSE_INVALID_VALUE, "abc ");
	TEST_ERROR(PARSE_INVALID_VALUE, "true ");
	TEST_ERROR(PARSE_OK, "true ");
}
static void test_parse() {
	test_parse_null();
	test_parse_true();
	test_parse_false();
	test_error();
	//test_parse_number();
}
int main() {
#if 1
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
#endif
}
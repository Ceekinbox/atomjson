
#include <iostream>
#include <cstdlib>
#include <string>

#include "atomjson.h"
using namespace atom;

static int main_ret = 0;
static int test_count = 0;
static int test_pass = 0;
inline void EXPECT_EQ_BASE(bool equality, int expect,
							int actual, std::string format) {
	test_count++;
	if (equality)
		test_pass++;
	else {
		fprintf(stderr, "%s:%d: expect: %d actual: %d \n", __FILE__, __LINE__, expect, actual);
		main_ret = 1;
	}
};
inline void EXPECT_EQ_INT(int expect, int actual) {
	EXPECT_EQ_BASE(expect == actual, expect, actual, "%d");
}

static void test_parse_null() {
	CJsonValue v;
	v.type = ATOM_TRUE;

	EXPECT_EQ_INT(PARSE_OK, parse(&v, "null"));
	//EXPECT_EQ_INT(ATOM_TRUE, get_type(&v));
}

static void test_parse_true() {
	CJsonValue v;
	v.type = ATOM_TRUE;
	EXPECT_EQ_INT(ATOM_TRUE, get_type(&v));
	EXPECT_EQ_INT(PARSE_OK, parse(&v, "true"));
}
static void test_parse() {
	test_parse_null();
	test_parse_true();
}
int main() {
	test_parse();
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	return main_ret;
}
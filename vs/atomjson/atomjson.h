/****************************************
#	JSON-text = ws value ws				#
#	ws = *(%x20 / %x09 / %x0A / %x0D)	#
#	value = null / false / true			#
#	null  = "null"						#
#	false = "false"						#
#	true  = "true"						#
#***************************************#
#   version v0.1						#
****************************************/
#pragma once
#ifndef ATOMJSON_H__
#define ATOMJSON_H__
#include <assert.h>
#endif /* LEPTJSON_H__ */

namespace atom {
	//七种枚举类型
	typedef enum{
		ATOM_NULL, ATOM_FALSE, ATOM_TRUE,
		ATOM_NUMBER, ATOM_STRING, ATOM_ARRAY, ATOM_OBJECT
	} data_type;

	enum {

	};
	//json的数据结构
	class CJsonValue
	{
	public:
		data_type type;
		double n;
	};

	int parse(CJsonValue *v, const char* json);

	enum {
		PARSE_OK = 0,
		PARSE_EXPECT_VALUE,
		PARSE_INVALID_VALUE,
		PARSE_ROOT_NOT_SINGULAR
	};
	data_type get_type(const CJsonValue* v);

	double get_number(const CJsonValue* v); 
}





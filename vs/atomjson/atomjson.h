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
	//����ö������
	typedef enum{
		ATOM_NULL, ATOM_FALSE, ATOM_TRUE,
		ATOM_NUMBER, ATOM_STRING, ATOM_ARRAY, ATOM_OBJECT
	} data_type;

	enum {

	};
	//json�����ݽṹ
	class CJsonValue
	{
	public:
		union {
			struct {size_t len;char* s;}s;
			double n;
		};
		
		data_type type;


		data_type get_type();

		double get_number();
		void set_number(double n);

		const char* get_string();
		size_t get_string_length();
		void set_string(const char* s, size_t len);

		bool get_boolen();
		void set_boolen(bool b);

		void _init();
		void _free();
	};

	class _context{
	public:
		const char* json;
		char* stack;
		size_t size, top;

		void* _push(size_t size);
		void* _pop(size_t size);

	};
	int parse(CJsonValue *v, const char* json);

	enum {
		PARSE_OK = 0,
		PARSE_EXPECT_VALUE,
		PARSE_INVALID_VALUE,
		PARSE_ROOT_NOT_SINGULAR,
		PARSE_NUMBER_TOO_BIG,
		PARSE_MISS_QUOTATION_MARK,
		PARSE_INVALID_STRING_ESCAPE,
		PARSE_INVALID_STRING_CHAR

	};
#if 0 data_type get_type(const CJsonValue* v);

	double get_number(const CJsonValue* v); 
#endif
}





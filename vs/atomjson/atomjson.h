
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

	struct member;
	//json的数据结构
	class CJsonValue
	{
	public:
		union {
			struct {size_t len;char* s;}s;
			struct {CJsonValue* e ; size_t size; }a;
			struct {member* m ; size_t size; }o;
			double n;
		};
		
		data_type type;


		data_type get_type();

		double get_number();
		void set_number(double n);

		const char* get_string();
		size_t get_string_length();
		void set_string(const char* s, size_t len);

		size_t get_array_size();
		CJsonValue get_array_element(int index);

		size_t get_object_size();
		const char* get_object_key(size_t index);
		size_t get_object_key_length(size_t index);
		CJsonValue get_object_value(size_t index);

		bool get_boolen();
		void set_boolen(bool b);

		void _init();
		void _free();
	};
	struct member {
		char* key;
		size_t keylen;
		CJsonValue v;
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
		PARSE_INVALID_STRING_CHAR,
		PARSE_INVALID_UNICODE_HEX,
		PARSE_INVALID_UNICODE_SURROGATE,
		PARSE_MISS_COMMA_OR_SQUARE_BRACKET,
		PARSE_MISS_COLON,
		PARSE_MISS_KEY,
		PARSE_MISS_COMMA_OR_CURLY_BRACKET

	};
#if 0 data_type get_type(const CJsonValue* v);

	double get_number(const CJsonValue* v); 
#endif
}





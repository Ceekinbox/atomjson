#include "atomjson.h"
#include <assert.h>
#include <stdlib.h>

#define EXPECT(c,ch) { assert(*c->json == (ch));c->json++;}
namespace atom {
	typedef struct {
		const char* json;
	}_context;
	
	//处理空格
	static void _parse_whitespace(_context* c) {
		const char* p = c->json;
		while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
			p++;
		c->json = p;
	}

	//处理null
	static int _parse_null(_context* c, CJsonValue* v) {
		EXPECT(c, 'n');
		if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
			return PARSE_INVALID_VALUE;
		c->json += 3;
		v->type = ATOM_NULL;
		return PARSE_OK;
	}
	//处理true
	static int _parse_true(_context* c, CJsonValue* v) {
		EXPECT(c, 't');
		if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
			return PARSE_INVALID_VALUE;
		c->json += 3;
		v->type = ATOM_TRUE;
		return PARSE_OK;
	}
	//处理false
	static int _parse_false(_context* c, CJsonValue* v) {
		EXPECT(c, 'f');
		if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
			return PARSE_INVALID_VALUE;
		c->json += 4;
		v->type = ATOM_FALSE;
		return PARSE_OK;
	}
	//处理数字
	static int _parse_number(_context* c, CJsonValue* v) {
		char* end;
		v->n = strtod(c->json, &end);
		if (c->json == end) {
			return PARSE_INVALID_VALUE;
		}
		c->json = end;
		v->type = ATOM_NUMBER;
		return PARSE_OK;
	}

	static int _parse_value(_context* c, CJsonValue* v) {
		switch (*c->json) {
		case 'n':	return _parse_null(c, v);
		case 'f':	return _parse_false(c, v);
		case 't':	return _parse_true(c, v);
		case '\0':	return PARSE_EXPECT_VALUE;
		default:	return _parse_number(c, v);
		}
	}


	int parse(CJsonValue* v, const char* json) {
		_context c;
		int ret;

		c.json = json;
		v->type = ATOM_NULL;
		_parse_whitespace(&c);
		if ((ret = _parse_value(&c, v)) == PARSE_OK) {
			_parse_whitespace(&c);
			if (*c.json != '\0') {
				v->type = ATOM_NULL;
				ret = PARSE_ROOT_NOT_SINGULAR;
			}
		}
		return ret;
	}

	data_type get_type(const CJsonValue* v) {
		return v->type;
	}

	double get_number(const CJsonValue* v){
		assert(v->type == ATOM_NUMBER);
		return v->n;
	}
}

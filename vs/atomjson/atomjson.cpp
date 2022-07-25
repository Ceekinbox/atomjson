#include "atomjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>  

#define EXPECT(c,ch) do{ assert(*c->json == (ch));c->json++;}while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

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
	static int _parse_literal(_context* c, CJsonValue* v,data_type type,const char* literal) {
		EXPECT(c, literal[0]);
		size_t i;
		for (i = 0; literal[i + 1]; i++) {
			if (c->json[i] != literal[i + 1])
				return PARSE_INVALID_VALUE;
		}
		c->json += i;
		v->type = type;
		return PARSE_OK;
	}
	//处理数字
	static int _parse_number(_context* c, CJsonValue* v) {
		const char* p = c->json;
		if (*p == '-') p++;
		if (*p == '0') p++;
		else {
			if (!ISDIGIT1TO9(*p)) return PARSE_INVALID_VALUE;
			for (p++; ISDIGIT(*p); p++);
		}
		if (*p == '.') {
			p++;
			if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
			for (p++; ISDIGIT(*p); p++);
		}
		if (*p == 'e' || *p == 'E') {
			p++;
			if (*p == '+' || *p == '-') p++;
			if (!ISDIGIT(*p)) return PARSE_INVALID_VALUE;
			for (p++; ISDIGIT(*p); p++);
		}
		errno = 0;
		v->n = strtod(c->json, NULL);
		if (errno == ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL))
			return PARSE_NUMBER_TOO_BIG;
		v->type = ATOM_NUMBER;
		c->json = p;
		return PARSE_OK;
	}


	static int _parse_value(_context* c, CJsonValue* v) {
		switch (*c->json) {
		case 'n':	return _parse_literal(c, v,ATOM_NULL,"null");
		case 'f':	return _parse_literal(c, v,ATOM_FALSE,"false");
		case 't':	return _parse_literal(c, v,ATOM_TRUE,"true");
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

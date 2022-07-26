#include "atomjson.h"
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>  
#include <cstring>

#ifndef ATOM_PARSE_STACK_INIT_SIZE
#define ATOM_PARSE_STACK_INIT_SIZE 256
#endif

#define EXPECT(c,ch) do{ assert(*c->json == (ch));c->json++;}while(0)
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

#define STRING_ERROR(ret) do{ c->top = head; return ret; }while(0)

namespace atom {
	
	
	//处理空格
	static void _parse_whitespace(_context* c) {
		const char* p = c->json;
		while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
 			p++;
		c->json = p;
	}

#if 0
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
#endif

	//处理true false null
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
	//处理字符串
	//unicode UTF-8
	void inline PUTC(_context* c, char ch) {
		*(char*)c->_push(sizeof(char)) = ch;
	}
	void inline PUTC_UTF(_context* c, unsigned ch) {
		*(char*)c->_push(sizeof(char)) = ch;
	}
	static const char* _parse_hex4(const char* p, unsigned* u) {
		char* end;
		char ch = *p;
		if (!((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')))
			return nullptr;
		*u = strtol(p,&end,16);
		return end == p + 4 ? end : nullptr;
	}
	static void _encode_utf8(_context* c,unsigned u) {
		if (u <= 0x7F)
			PUTC_UTF(c, u & 0xFF);
		else if (u <= 0x7FF) {
			PUTC_UTF(c, 0xC0 | ((u >> 6) & 0xFF));
			PUTC_UTF(c, 0x80 | (u & 0x3F));
		}
		else if (u <= 0xFFFF) {
			PUTC_UTF(c, 0xE0 | ((u >> 12) & 0xFF));
			PUTC_UTF(c, 0x80 | ((u >> 6) & 0x3F));
			PUTC_UTF(c, 0x80 | (u & 0x3F));
		}
		else {
			assert(u <= 0x10FFFF);
			PUTC_UTF(c, 0xF0 | ((u >> 18) & 0xFF));
			PUTC_UTF(c, 0x80 | ((u >> 12) & 0x3F));
			PUTC_UTF(c, 0x80 | ((u >> 6) & 0x3F));
			PUTC_UTF(c, 0x80 | (u & 0x3F));
		}
	}

	static int _parse_string_raw(_context* c, char** str, size_t *len) {
		size_t head = c->top;
		const char* p;
		unsigned u, u2;
		EXPECT(c, '\"');
		p = c->json;

		for (;;) {
			char ch = *p++;
			switch (ch) {
			case '\"':
				*len = c->top - head;
				*str = (char*)c->_pop(*len);
				c->json = p;
				return PARSE_OK;
			case '\\':
				switch (*p++) {
				case '\"': PUTC(c, '\"'); break;
				case '\\': PUTC(c, '\\'); break;
				case '/':  PUTC(c, '/');  break;
				case 'b':  PUTC(c, '\b'); break;
				case 'f':  PUTC(c, '\f'); break;
				case 'n':  PUTC(c, '\n'); break;
				case 'r':  PUTC(c, '\r'); break;
				case 't':  PUTC(c, '\t'); break;
				case 'u':
					if ((p = _parse_hex4(p, &u)) == nullptr)
						STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
					if (u >= 0xD800 && u <= 0xDBFF) {
						if (*p != '\\')
							STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
						p++;
						if (*p != 'u')
							STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
						p++;
						if ((p = _parse_hex4(p, &u2)) == nullptr)
							STRING_ERROR(PARSE_INVALID_UNICODE_HEX);
						if (u2 < 0xDC00 || u2 > 0xDFFF)
							STRING_ERROR(PARSE_INVALID_UNICODE_SURROGATE);
						u = 0x10000 + ((u - 0xD800) << 10) + (u2 - 0xDC00);
					}
					_encode_utf8(c, u);
					break;
				default:
					c->top = head;
					return PARSE_INVALID_STRING_ESCAPE;
				}
				break;
			case '\0':
				c->top = head;
				return PARSE_MISS_QUOTATION_MARK;
			default:
				if ((unsigned char)ch < 0x20) {
					c->top = head;
					return PARSE_INVALID_STRING_CHAR;
				}
				PUTC(c, ch);
			}
		}
	}

	static int _parse_string(_context* c, CJsonValue* v) {
		int ret;
		char* str;
		size_t len;
		if((ret = _parse_string_raw(c,&str,&len)) == PARSE_OK)
			v->set_string(str, len);
		return ret;
	}
	
	static int _parse_value(_context* c, CJsonValue* v);
	//处理array
	static int _parse_array(_context* c, CJsonValue* v) {
		size_t head = c->top;//保存起始点
		size_t size = 0;
		int ret;
		EXPECT(c, '[');
		_parse_whitespace(c);
		if (*c->json == ']') {
			c->json++;
			v->type = ATOM_ARRAY;
			v->a.e = nullptr;
			v->a.size = 0;
			return PARSE_OK;
		}
		for (;;) {
			CJsonValue element;
			element._init();
			if ((ret = _parse_value(c, &element)) != PARSE_OK)
				break;
			memcpy(c->_push(sizeof(CJsonValue)),&element,sizeof(CJsonValue));
			size++;
			_parse_whitespace(c);
			if (*c->json == ',') {
				c->json++;
				_parse_whitespace(c);
			}
			else if (*c->json == ']') {
				c->json++;
				v->type = ATOM_ARRAY;
				v->a.size = size;
				size = size * sizeof(CJsonValue);
				memcpy(v->a.e = (CJsonValue*)malloc(size),c->_pop(size),size);
				return PARSE_OK;
			}
			else {
				ret = PARSE_MISS_COMMA_OR_SQUARE_BRACKET;
				break;
			}
		}
		for (int i = 0; i < size; i++) {
			CJsonValue* p = (CJsonValue*)c->_pop(sizeof(CJsonValue));
			p->_free();
		}
		return ret;
	}
	//处理object
	static int _parse_object(_context* c, CJsonValue* v) {
		size_t size;
		member m;
		int ret;
		EXPECT(c, '{');
		_parse_whitespace(c);
		if (*c->json == '}') {
			c->json++;
			v->type = ATOM_OBJECT;
			v->o.m = 0;
			v->o.size = 0;
			return PARSE_OK;
		}
		m.key = nullptr;
		size = 0;
		m.keylen = 0;
		for (;;) {
			if (*c->json == '"') {
				ret = PARSE_MISS_KEY;
				break;
			}
			/*获取key值*/
			char* str;
			m.v._init(); 
			if ((ret = _parse_string_raw(c,&str,&m.keylen)) != PARSE_OK)
				break;
			memcpy(m.key = (char*)malloc(m.keylen+1),str,m.keylen);//
			m.key[m.keylen] = '\0';
			/**********/
			_parse_whitespace(c);
			if (*c->json != ':') {
				ret = PARSE_MISS_COLON;
				break;
			}
			c->json++;
			_parse_whitespace(c);
			if ((ret = _parse_value(c, &m.v)) != PARSE_OK) 
				break;
			memcpy(c->_push(sizeof(member)), &m, sizeof(member));
			size++;
			m.key = nullptr;
			if (*c->json == ',') {
				c->json++;
				break;
			}
			else if (*c->json == '}') {

				c->json++;
				v->type = ATOM_OBJECT;
				v->o.size = size;
				size = size * sizeof(member);
				memcpy(v->o.m = (member* )malloc(size), c->_pop(size), size);
				return PARSE_OK;
			}
			else {
				ret = PARSE_MISS_COMMA_OR_CURLY_BRACKET;
				break;
			}

		}
		free(m.key);
		for (int i = 0; i < size; i++) {
			member* m = (member*)c->_pop(sizeof(member));
			free(m->key);
			m->v._free();
		}
		v->type = ATOM_NULL;
		return ret;
	}
	static int _parse_value(_context* c, CJsonValue* v) {
		switch (*c->json) {
		case 'n':	return _parse_literal(c, v,ATOM_NULL,"null");
		case 'f':	return _parse_literal(c, v,ATOM_FALSE,"false");
		case 't':	return _parse_literal(c, v,ATOM_TRUE,"true");
		case '"':	return _parse_string(c,v);
		case '[':	return _parse_array(c, v);
		case '\0':	return PARSE_EXPECT_VALUE;
		default:	return _parse_number(c, v);
		}
	}


	int parse(CJsonValue* v, const char* json) {
		_context c;
		int ret;
		assert(v != nullptr);

		//可以用构造函数初始化
		c.json = json;
		c.stack = nullptr;
		c.size = 0;
		c.top = 0;
		//*****

		v->_init();
		_parse_whitespace(&c);
		if ((ret = _parse_value(&c, v)) == PARSE_OK) {
			_parse_whitespace(&c);
			if (*c.json != '\0') {
				v->type = ATOM_NULL;
				ret = PARSE_ROOT_NOT_SINGULAR;
			}
		}

		assert(c.top == 0);
		free(c.stack);

		return ret;
	}

	void* _context::_push(size_t size) {
		void* ret;
		assert(size > 0);
		if (top + size >= this->size) {
			if (this->size == 0) 
				this->size = ATOM_PARSE_STACK_INIT_SIZE;
			while (top + size >= this->size)
				this->size += this->size >> 1;
			stack = (char*)realloc(stack, this->size);
		}
		ret = stack + top;
		top += size;
		return ret;
	}
	void* _context::_pop(size_t size) {
		assert(top >= size);
		return stack + (top -= size);
	}
	//在parse函数中释放内存
	//todo：	1.使用c++ 的 new 和 delete 管理
	//		2.整理_context类的声明

	data_type CJsonValue::get_type() {
		return type;}

	double CJsonValue::get_number(){
		assert(this != nullptr && type == ATOM_NUMBER);
		return n;
	}
	void CJsonValue::set_number(double n) {
		_free();
		type = ATOM_NUMBER;
		this->n = n;
	}

	void CJsonValue::set_string(const char* s, size_t len) {
		assert(s != nullptr || len == 0);
		_free();
		this->s.s = (char*)malloc(len + 1);
		memcpy(this->s.s, s, len);
		this->s.s[len] = '\0';
		this->s.len = len;
		type = ATOM_STRING;
	}

	size_t CJsonValue::get_object_size() {
		assert(this != nullptr && type == ATOM_OBJECT);
		return o.size;
	}
	const char* CJsonValue::get_object_key(size_t index) {
		assert(this != nullptr && type == ATOM_OBJECT);
		return o.m[index].key;
	}
	size_t CJsonValue::get_object_key_length(size_t index) {
		assert(this != nullptr && type == ATOM_OBJECT);
		return o.m[index].keylen;
	}
	CJsonValue CJsonValue::get_object_value(size_t index) {
		assert(this != nullptr && type == ATOM_OBJECT);
		return o.m[index].v;
	}
	const char* CJsonValue::get_string() {
		assert(this != nullptr && type == ATOM_STRING);
		return s.s;
	}

	size_t CJsonValue::get_string_length() {
		assert(this != nullptr && type == ATOM_STRING);
		return s.len;
	}

	size_t CJsonValue::get_array_size() {
		assert(this != nullptr && type == ATOM_ARRAY);
		return a.size;
	}
	CJsonValue CJsonValue::get_array_element(int index) {
		assert(this != nullptr && type == ATOM_ARRAY);
		assert(index >= 0 && index < a.size);
		return a.e[index];
	}
	bool CJsonValue::get_boolen() {
		assert(this != nullptr && (type == ATOM_TRUE || type == ATOM_FALSE));
		if (type == ATOM_TRUE)return true;
		else return false;
	}
	void CJsonValue::set_boolen(bool b) {
		_free();
		if (b) type = ATOM_TRUE;
		else type = ATOM_FALSE;
	}

	void inline CJsonValue::_free() {
		assert(this != nullptr);
		int i;
		if (type == ATOM_STRING)
			free(s.s);
		else if (type == ATOM_ARRAY) {
			for (i = 0; i < a.size; i++)
				a.e[i]._free();
			free(a.e);
		}
		else if (type == ATOM_OBJECT) {
			for (i = 0; i < o.size; i++) {
				free(o.m[i].key);
				o.m[i].v._free();
			}
			free(o.m);
		}
		type = ATOM_NULL;
	}

	void inline CJsonValue::_init() {
		type = ATOM_NULL;
	}


}

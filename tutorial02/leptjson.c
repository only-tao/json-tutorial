#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include<string.h>
#include <math.h>
#include <errno.h>
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define LEPT_PARSE_T_F_NULL(c,v,flag_str,ret) \
    do\
    {\
        EXPECT(c,flag_str[0]);\
        const char* flag_s = flag_str;\
        int size = strlen(flag_s);\
        int i=0;\
        while(i+1<size){\
            if(c->json[i]!=flag_s[i+1]){\
                return LEPT_PARSE_INVALID_VALUE;\
            }\
            i++;\
        }\
        c->json += size;\
        v->type = ret;\
        return LEPT_PARSE_OK;\
    }while(0)
#define ISDIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch) ((ch)>'0' && (ch)<='9')
#define ISPREFIXNUM(ch) (ISDIGIT(ch) || (ch) == '-')
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v,const char* flag_str,lept_type ret){
    LEPT_PARSE_T_F_NULL(c,v,flag_str,ret);
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    /* \TODO validate number */// 格式校验! 
    //EXPECT(c, ISPREFIXNUM(c));
	/*if (!ISPREFIXNUM(*c->json)) {
		return LEPT_PARSE_INVALID_VALUE;
	}*/
    // 整数部分如果是 0 开始，只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9）
    char* c_temp = c->json;
    //lept_context* c_temp = c;
    if (*c->json == '-') {
		c->json++;
    }
    if (*c->json == '0') {
        c->json++;
        //之后只能为 null, . 
        if (*c->json != '.' && *c->json != '\0') {
            return LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    else {
		if (ISDIGIT1TO9(*c->json)) {
			c->json++;
			while (ISDIGIT(*c->json)) {
				c->json++;
			}
		}
		else {
			return LEPT_PARSE_INVALID_VALUE;
		}
	}// 接下来处理小数部分
    if (*c->json == '.') {
        c->json++;
        if (ISDIGIT(*c->json)) {
            c->json++;
            while (ISDIGIT(*c->json)) {
                c->json++;
            }
            if(ISDIGIT1TO9(*c->json)){
                c->json++;
            }
        }
        else {
            return LEPT_PARSE_INVALID_VALUE;
        }
            // 如果有 . 则一定要有 之后的小数
    }
    
    if (*c->json == 'e' || *c->json == 'E') {// 有指数部分
        c->json++;
        if (*c->json == '-' || *c->json == '+') {
            c->json++;
        }
        if (ISDIGIT(*c->json)) {
            while (ISDIGIT(*c->json)) {
                c->json++;
            }
        }
        else {
            return LEPT_PARSE_INVALID_VALUE;
        }
    }
    c->json = c_temp;
    errno = 0;
    v->n = strtod(c->json, &end);
    
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    //if (errno == ERANGE && isinf(v->n)) {
    if(errno == ERANGE &&(v->n == HUGE_VAL || v->n==-HUGE_VAL)){
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c,v,"true",LEPT_TRUE);
        case 'f':  return lept_parse_literal(c,v,"false",LEPT_FALSE);
        case 'n':  return lept_parse_literal(c,v,"null",LEPT_NULL);
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}

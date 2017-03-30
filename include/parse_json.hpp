#ifndef PARSE_JSON_HPP__
#define PARSE_JSON_HPP__

#include "common.hpp"

#ifdef __cplusplus
extern "C"
{
#endif

#include <json-c/json.h>

typedef struct {
    json_tokener    *tok;
    FILE            *fpJson;
    char            *pBuf;
    size_t          szBuf;
    bool            bPosRst;
    int             nPrevPos;
    int             nLastPos;
} json_parser;

void print_json_value(json_object *jobj);
void json_parse_array(json_object *jobj, char *key);
void json_parse(json_object *jobj);
void json_retrieve_scheme(json_object *jobj);
json_object* json_parser_getJobj(json_parser *jParser);
json_parser* json_parser_init(const char *pJsonFileName);
void json_parser_close(json_parser* jParser);

#ifdef __cplusplus
}
#endif

#endif // PARSE_JSON_HPP__

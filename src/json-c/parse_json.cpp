#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include "parse_json.hpp"

#define BUFLEN 256

static bool bDebug = false;

void print_json_value(json_object *jobj) {
    enum json_type type;
    type = json_object_get_type(jobj);
    printf("[%s] ", json_type_to_name(type));    
    switch(type) {
        case json_type_null:
            printf("%s\n", json_object_get_string(jobj));
            break;
        case json_type_boolean:
            printf("%s\n", json_object_get_boolean(jobj) ? "true" : "false");
            break;
        case json_type_int:
            printf("%d\n", json_object_get_int(jobj));
            break;
        case json_type_double:
            printf("%lf\n", json_object_get_double(jobj));
            break;
        case json_type_string:
            printf("%s\n", json_object_get_string(jobj));
            break;
        default:
            /* These are not value type */
            printf("\n");
            break;
    }
}

void json_parse_array(json_object *jobj, char *key) {
    void json_parse(json_object *jobj);
    enum json_type type;

    json_object *jarray = jobj;
    if (key) {
        json_object_object_get_ex(jobj, key, &jarray);
    }
    printf("test: %p", (void *)key);
    int arraylen = json_object_array_length(jarray);
    printf("Array length: %d\n", arraylen);

    json_object *jvalue;
    for (int i=0; i < arraylen; i++) {
        jvalue = json_object_array_get_idx(jarray, i);
        type = json_object_get_type(jvalue);
        if (type == json_type_array) {
            json_parse_array(jvalue, NULL);
        }
        else if(type != json_type_object) {
            printf("value[%d]: ", i);
            print_json_value(jvalue);
        }
        else {
            json_parse(jvalue);
        }
    }
}

void json_parse(json_object *jobj) {
    enum json_type type;
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);
        printf("type: %s ", key);
        switch(type) {
            case json_type_null:
            case json_type_boolean:
            case json_type_int:
            case json_type_double:
            case json_type_string:
                print_json_value(val);
                break;
            case json_type_object:
                json_object_object_get_ex(jobj, key, &jobj);
                json_parse(jobj);
                break;
            case json_type_array:
                json_parse_array(jobj, key);
                break;
        }
    }
}

void json_retrieve_scheme(json_object *jobj) {
    enum json_type type, subtype;
    json_object_object_foreach(jobj, key, val) {
        type = json_object_get_type(val);
        printf("type: %s[%s]", key, json_type_to_name(type));
        switch(type) {
            case json_type_object:
                printf("\n%s\n", json_object_to_json_string_ext(val, JSON_C_TO_STRING_PRETTY));
                break;
            default:
                printf("\n");
                break;
        }
    }
}

json_object* json_getJobj(json_tokener *restrict tok, char *restrict pStr, size_t szStrLen, int *restrict nOffset, bool bVerbose) {

    json_object *jobj = NULL;
    enum json_tokener_error jerr;
    
    if (tok == NULL || pStr == NULL || nOffset == NULL) {
        return NULL;
    }
    jobj = json_tokener_parse_ex(tok, pStr, szStrLen);
    jerr = json_tokener_get_error(tok);
    if (bVerbose) {
        printf("[%s:%d] %s\n", __func__, __LINE__, json_tokener_error_desc(jerr));
    }
    if (jerr != json_tokener_success)
    {
        if (jobj != NULL) {
            printf("[%s:%d] Should not return partial object: %s\n", __func__, __LINE__, json_tokener_error_desc(jerr));
        }
        *nOffset = -1;
    }
    else if (jobj != NULL)
    {
        /* Success in parsing JSON */
        *nOffset = tok->char_offset;
    }
    else
    {
        printf("[%s:%d] Unexpected JSON Parsing Error\n", __func__, __LINE__);
        *nOffset = 0;
    }

    return jobj;
}

size_t trim_trailingWhiteSpace(char *pStr, size_t szStrLen) {
    int nPos = 0;
    if (pStr == NULL || szStrLen == 0){
        return 0;
    }
    nPos = szStrLen - 1;
    while(nPos > 0 && isspace(pStr[nPos])) nPos--;

    if (nPos < 0) {
        nPos = -1;
    }
    return nPos + 1;
}

char* get_strBuf(char *restrict pBuf, size_t *restrict pszBuf, int *restrict pnLastPos, char *restrict pNewline, size_t szNewline) {
    const size_t szIncBuf = BUFLEN;
    size_t szBufRemain = 0; 
    char *pNew = NULL;
    int nLastPos = 0;
    
    if (pszBuf == NULL || pnLastPos == NULL || pNewline == NULL || szNewline == 0) {
        printf("[%s:%d] param error (pszBuf: %p, pnLastPos: %p, pNewline: %p, szNewline: %lu)", \
            __func__, __LINE__, (void *)pszBuf, (void *)pnLastPos, (void *)pNewline, szNewline);
        return NULL;
    }
        
    if (pBuf == NULL) {
        szBufRemain = 0;
        nLastPos = 0; 
    } else {
        nLastPos = *pnLastPos;
        szBufRemain = *pszBuf - nLastPos;
    }   
        
    if (szBufRemain < szNewline) {
        size_t szNewBuf = nLastPos + szIncBuf;
    
        if (szNewline > szNewBuf) {
            szNewBuf = szNewline;
        }
        pNew = (char *)realloc(pBuf, sizeof(char) * szNewBuf);
        if (pNew != NULL) {
            *pszBuf = szNewBuf;
        } else { 
            printf("[%s:%d] pNew realloc error", __func__, __LINE__);
        } 
    } else {
        pNew = pBuf;
    } 
        
    memcpy(pNew + nLastPos, pNewline, sizeof(char) * szNewline);
    *pnLastPos = nLastPos + szNewline;
    
    return pNew; 
}

json_object* json_parser_getJobj(json_parser *jParser) {
    char strBuf[BUFLEN];
    size_t len = 0;
    ssize_t read = 0;
    json_object *jObj = NULL;
    int nOffset = -1;

    if (jParser) {
        while(fgets(strBuf, BUFLEN, jParser->fpJson) != NULL) {
            json_object *jObjTmp = NULL;
            /* 
             * At first, trim trailing whitespaces from string 
             * If there is no line, skip to the next line
             */
            size_t szStrTrim = trim_trailingWhiteSpace(strBuf, strlen(strBuf));
            if (szStrTrim == 0) continue;
            /*
             * JSON tokener does not take care of string buffer.
             * Therefore, we need to keep the entire string until JSON tokener returns JSON object.
             * CAVEAT: For each parsing step, we are going to feed each line instead of the entire string.
             */
            if (jParser->bPosRst) {
                jParser->nPrevPos = 0;
                jParser->bPosRst = false;
            } else {
                jParser->nPrevPos = jParser->nLastPos;
            }
            jParser->pBuf = get_strBuf(jParser->pBuf, &jParser->szBuf, &jParser->nLastPos, strBuf, szStrTrim);
            jObjTmp = json_getJobj(jParser->tok, jParser->pBuf + jParser->nPrevPos, jParser->nLastPos - jParser->nPrevPos, &nOffset, bDebug);
            if (jObjTmp != NULL) {
                /*
                 * If JSON object is created by JSON tokener,
                 * we do not need to keep the original string.
                 * Restore buffer space for the following strings.
                 */
                int nRemain = jParser->nLastPos - jParser->nPrevPos - nOffset;
                memmove(jParser->pBuf, jParser->pBuf + jParser->nPrevPos + nOffset, nRemain);
                jParser->nLastPos = nRemain;

                jObj = jObjTmp;
                json_tokener_reset(jParser->tok);
                jParser->bPosRst = true;
                break;
            }
        }
    }
    return jObj;
}

json_parser* json_parser_init(const char *pJsonFileName) {
    json_parser tmp = {NULL, NULL, NULL, 0, true, 0, 0};
    json_parser *jParser = NULL;
    FILE        *fp = fopen(pJsonFileName, "r");

    if (fp != NULL) {
        jParser = (json_parser *)malloc(sizeof(json_parser));
        if (jParser) {
            memcpy(jParser, &tmp, sizeof(json_parser));
            jParser->fpJson = fp;
            if ((jParser->tok = json_tokener_new()) != NULL) {
                /* Create JSON tokener : parsing rule is based on strict mode */
                json_tokener_set_flags(jParser->tok, JSON_TOKENER_STRICT);
                /* read characters from JSON file */
                jParser->bPosRst = 1;
            } else {
                printf("[%s:%d] Error in create json tokener\n", __func__, __LINE__);
                json_parser_close(jParser);
                jParser = NULL;
            }
        }
    } else {
        printf("[%s:%d] Error in opening %s\n", __func__, __LINE__, pJsonFileName);
    }
    return jParser;
}

void json_parser_close(json_parser* jParser) {
    if (jParser) {
        if (jParser->tok) json_tokener_free(jParser->tok);
        if (jParser->fpJson) fclose(jParser->fpJson);
        if (jParser->pBuf) free(jParser->pBuf);
        free(jParser);
    }
}
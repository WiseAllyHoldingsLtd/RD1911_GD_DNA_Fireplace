#pragma once

#ifndef UNITTESTS
#include "cJSON.h"
#else
#include <cstdint>

typedef struct cJSON
{
    char *valuestring;
    int valueint;
} cJSON;

typedef int cJSON_bool;

cJSON * cJSON_CreateObject(void);
cJSON * cJSON_Parse(const char * buf);
cJSON * cJSON_GetObjectItem(cJSON * obj, const char * key);
int cJSON_IsObject(cJSON * obj);

char * cJSON_PrintUnformatted(cJSON * obj);

cJSON * cJSON_AddStringToObject(cJSON * obj, const char * key, const char * val);
cJSON * cJSON_AddNullToObject(cJSON * obj, const char * key);
cJSON * cJSON_AddBoolToObject(cJSON * obj, const char * key, const cJSON_bool boolean);
cJSON * cJSON_AddObjectToObject(cJSON * obj, const char * key);
cJSON * cJSON_AddNumberToObject(cJSON * obj, const char * key, double val);
void cJSON_Delete(cJSON * obj);
#endif

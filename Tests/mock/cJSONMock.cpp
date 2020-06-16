#include "cJSON.hpp"

cJSON * cJSONObj;
char cJSONStr[] = "{ Implemented: No }";

cJSON * cJSON_CreateObject(void)
{
  return cJSONObj;
}

cJSON * cJSON_Parse(const char * buf)
{
  return cJSONObj;
}

cJSON * cJSON_GetObjectItem(cJSON * obj, const char * key)
{
  return cJSONObj;
}

int cJSON_IsObject(cJSON * obj)
{
  return 1;
}


char * cJSON_PrintUnformatted(cJSON * obj)
{
  return cJSONStr;
}

cJSON * cJSON_AddStringToObject(cJSON * obj, const char * key, const char * val)
{
  return cJSONObj;
}

cJSON * cJSON_AddNullToObject(cJSON * obj, const char * key)
{
  return cJSONObj;
}

cJSON * cJSON_AddBoolToObject(cJSON * obj, const char * key, const cJSON_bool boolean)
{
  return cJSONObj;
}

cJSON * cJSON_AddObjectToObject(cJSON * obj, const char * key)
{
  return cJSONObj;
}

cJSON * cJSON_AddNumberToObject(cJSON * obj, const char * key, double val)
{
  return cJSONObj;
}

void cJSON_Delete(cJSON * obj)
{
}

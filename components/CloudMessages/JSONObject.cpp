#include "JSONObject.h"


JSONObject::JSONObject(void)
  : m_object(nullptr)
{
}

JSONObject::~JSONObject(void)
{
  if (objectExists())
  {
    cJSON_Delete(m_object);
    m_object = nullptr;
  }
}

bool JSONObject::create(void)
{
  bool wasCreated = false;

  if (!objectExists())
  {
    m_object = cJSON_CreateObject();
    wasCreated = objectExists();
  }
  else
  {
    cJSON_Delete(m_object);
    m_object = cJSON_CreateObject();
    wasCreated = objectExists();
  }

  return wasCreated;
}

bool JSONObject::parse(const char * jsonString)
{
  bool wasCreated = false;

  if (!objectExists())
  {
    m_object = cJSON_Parse(jsonString);
    wasCreated = objectExists();
  }

  return wasCreated;
}


bool JSONObject::addString(const char * key, const char * value)
{
  bool wasCreated = false;

  if (objectExists())
  {
    wasCreated = (cJSON_AddStringToObject(m_object, key, value) != nullptr);
  }

  return wasCreated;
}


bool JSONObject::addValue(const char * key, int32_t value)
{
  bool wasCreated = false;

  if (objectExists())
  {
    wasCreated = (cJSON_AddNumberToObject(m_object, key, value) != nullptr);
  }

  return wasCreated;
}


bool JSONObject::addNull(const char * key)
{
  bool wasCreated = false;

  if (objectExists())
  {
    wasCreated = (cJSON_AddNullToObject(m_object, key) != nullptr);
  }

  return wasCreated;
}


bool JSONObject::addBool(const char * key, bool value)
{
  bool wasCreated = false;

  if (objectExists())
  {
    wasCreated = (cJSON_AddBoolToObject(m_object, key, (value == true ? 1 : 0)) != nullptr);
  }

  return wasCreated;
}


bool JSONObject::addSubObject(const char * key)
{
  bool wasCreated = false;

  if (objectExists())
  {
    wasCreated = (cJSON_AddObjectToObject(m_object, key) != nullptr);
  }

  return wasCreated;
}


bool JSONObject::addStringToSubObject(const char * objectName, const char * key, const char * value)
{
  bool wasCreated =  false;

  if (subObjectExists(objectName))
  {
    cJSON * subObject = cJSON_GetObjectItem(m_object, objectName);
    wasCreated = (cJSON_AddStringToObject(subObject, key, value) != nullptr);
  }

  return wasCreated;
}


bool JSONObject::addValueToSubObject(const char * objectName, const char * key, int32_t value)
{
  bool wasCreated =  false;

  if (subObjectExists(objectName))
  {
    cJSON * subObject = cJSON_GetObjectItem(m_object, objectName);
    wasCreated = (cJSON_AddNumberToObject(subObject, key, value) != nullptr);
  }

  return wasCreated;
}


const char * JSONObject::getString(const char * key) const
{
  char * returnString = nullptr;

  if (objectExists())
  {
    cJSON * item = cJSON_GetObjectItem(m_object, key);

    if (item != nullptr)
    {
      returnString = item->valuestring;
    }
  }

  return returnString;
}


bool JSONObject::getValue(const char * key, int32_t & outValue) const
{
  bool wasFound;

  if (objectExists())
  {
    cJSON * item = cJSON_GetObjectItem(m_object, key);

    if (item != nullptr)
    {
      outValue = item->valueint;
      wasFound = true;
    }
  }

  return wasFound;
}


const char * JSONObject::getStringFromSubObject(const char * objectName, const char * key) const
{
  char * returnString = nullptr;

  if (subObjectExists(objectName))
  {
    cJSON * subObject = cJSON_GetObjectItem(m_object, objectName);
    cJSON * item = cJSON_GetObjectItem(subObject, key);

    if (item != nullptr)
    {
      returnString = item->valuestring;
    }
  }

  return returnString;
}


bool JSONObject::getValueFromSubObject(const char * objectName, const char * key, int32_t & outValue) const
{
  bool wasFound;

  if (subObjectExists(objectName))
  {
    cJSON * subObject = cJSON_GetObjectItem(m_object, objectName);
    cJSON * item = cJSON_GetObjectItem(subObject, key);

    if (item != nullptr)
    {
      outValue = item->valueint;
      wasFound = true;
    }
  }

  return wasFound;
}


char * JSONObject::printJSON(void) const
{
  char * jsonString = nullptr;

  if (objectExists())
  {
    jsonString = cJSON_PrintUnformatted(m_object);
  }

  return jsonString;
}


bool JSONObject::objectExists(void) const
{
  return (m_object != nullptr);
}


bool JSONObject::subObjectExists(const char * key) const
{
  bool doesExist = false;

  if (objectExists())
  {
    cJSON * subObject = cJSON_GetObjectItem(m_object, key);

    if (subObject != nullptr)
    {
      doesExist = (cJSON_IsObject(subObject) != 0);
    }
  }

  return doesExist;
}



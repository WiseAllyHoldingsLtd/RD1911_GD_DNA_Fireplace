#pragma once

#include <cstdint>
#include "cJSON.hpp"


class JSONObject
{
public:
  JSONObject(void);
  ~JSONObject(void);

  bool create(void);
  bool parse(const char * jsonString);

  bool addString(const char * key, const char * value);
  bool addValue(const char * key, int32_t value);
  bool addNull(const char * key);
  bool addBool(const char * key, bool value);
  bool addSubObject(const char * key);
  bool addStringToSubObject(const char * objectName, const char * key, const char * value);
  bool addValueToSubObject(const char * objectName, const char * key, int32_t value);

  const char * getString(const char * key) const;
  bool getValue(const char * key, int32_t & outValue) const;
  const char * getStringFromSubObject(const char * objectName, const char * key) const;
  bool getValueFromSubObject(const char * objectName, const char * key, int32_t & outValue) const;

  char * printJSON(void) const; /* Note: Caller is responsible for freeing char* memory. */

private:
  bool objectExists(void) const;
  bool subObjectExists(const char * key) const;

  cJSON * m_object;
};

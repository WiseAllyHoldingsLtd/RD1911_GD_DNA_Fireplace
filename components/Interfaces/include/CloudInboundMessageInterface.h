#pragma once


class CloudInboundMessageInterface
{
public:
  virtual ~CloudInboundMessageInterface(void) {}

  virtual bool parseJSONString(const char * jsonString) = 0;
};

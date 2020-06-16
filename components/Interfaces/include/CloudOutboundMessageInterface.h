#pragma once


class CloudOutboundMessageInterface
{
public:
  virtual ~CloudOutboundMessageInterface(void) {}

  virtual char * getJSONString(void) const = 0;
};

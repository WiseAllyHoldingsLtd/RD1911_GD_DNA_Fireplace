#include "HealthCheckRequest.h"


HealthCheckRequest::HealthCheckRequest(const char * gdid)
{
  m_message.create();
  m_message.addString("Gdid", gdid);
  m_message.addBool("DirectAppliance", true);
}

char * HealthCheckRequest::getJSONString(void) const
{
  return m_message.printJSON();
}

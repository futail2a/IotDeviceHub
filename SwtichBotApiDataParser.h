#ifndef SWITCHBOTAPIIF_H
#define SWITCHBOTAPIIF_H

#include <string>
#include <vector>
#include <dbus/dbus.h>

#include "SensorDataParser.h"

class WoSensorTHDataParser : public SensorDataParser
{
public:
  std::vector<uint8_t> parse_reply(DBusMessage* reply ) override;

private:
  std::vector<uint8_t> m_get_service_data(DBusMessageIter* variant_iter);
  std::vector<uint8_t> m_get_variant_byte_array(DBusMessageIter* variant_iter);
  void m_print_byte_array(const std::vector<uint8_t>& data);
};

#endif
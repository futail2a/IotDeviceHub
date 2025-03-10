#ifndef SWITCHBOTAPIIF_H
#define SWITCHBOTAPIIF_H

#include <string>
#include <vector>
#include <dbus/dbus.h>

#include "SensorDataParser.h"

constexpr uint8_t SERVICEDATA_LEN = 6;
constexpr uint8_t BIT_7_MASK = 0x80;
constexpr uint8_t BIT_0_6_MASK = 0x7f;

class WoSensorTHDataParser : public SensorDataParser
{
public:
  std::vector<uint8_t> parse_reply(DBusMessage* const reply ) override;
  void print_sensor_data(const std::vector<uint8_t>& data) override;
  std::string get_device_mac() override { return m_device_mac; }

private:
  std::vector<uint8_t> m_get_service_data(DBusMessageIter* const variant_iter);
  std::vector<uint8_t> m_get_variant_byte_array(DBusMessageIter* const variant_iter);
  void m_print_byte_array(const std::vector<uint8_t>& data);

  // Hardcord the MAC address of the device to be found
  // TODO: To find target adv data by service UUID
  const std::string m_device_mac = "XX:XX:XX:XX:XX:XX";
};

#endif
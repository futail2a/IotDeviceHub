#ifndef MOTION_SENSOR_DATA_HANDLER_H
#define MOTION_SENSOR_DATA_HANDLER_H

#include <string>
#include <vector>
#include <dbus/dbus.h>

#include "SensorDataHandler.h"

class MotionSensorDataHandler : public SensorDataHandler
{
public:
  void set_update_cb(UpdateCb cb) override { m_update_cb = cb; }
  std::vector<uint8_t> parse_reply(DBusMessage* const reply) override;
  std::string get_device_mac() override { return m_device_mac; }
  std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data) override;
  void update(DBusMessage* const reply) override;

private:
  std::vector<uint8_t> m_get_service_data(DBusMessageIter* const variant_iter);
  std::vector<uint8_t> m_get_variant_byte_array(DBusMessageIter* const variant_iter);
  void m_print_sensor_data(const std::vector<uint8_t>& data);
  UpdateCb m_update_cb;

  // Hardcord the MAC address of the device to be found
  // TODO: To find target adv data by service UUID
  const std::string m_device_mac = "";
};

#endif
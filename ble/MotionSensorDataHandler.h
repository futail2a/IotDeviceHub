#ifndef MOTION_SENSOR_DATA_HANDLER_H
#define MOTION_SENSOR_DATA_HANDLER_H

#include <string>
#include <vector>

#include "SensorDataHandler.h"

class MotionSensorDataHandler : public SensorDataHandler
{
public:
  void set_update_cb(UpdateCb cb) override { m_update_cb = cb; }
  std::string get_device_mac() override { return m_device_mac; }
  std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data) override;
  void update(std::vector<uint8_t> &data) override;

private:
  void m_print_sensor_data(const std::vector<uint8_t>& data);
  UpdateCb m_update_cb;

  // Hardcord the MAC address of the device to be found
  // TODO: To find target adv data by service UUID
  const std::string m_device_mac = "B0:E9:FE:55:04:12";
};

#endif
#ifndef WO_TH_SENSOR_DATA_HANDLER_H
#define WO_TH_SENSOR_DATA_HANDLER_H

#include <string>
#include <vector>

#include "SensorDataHandler.h"

constexpr uint8_t SERVICEDATA_LEN = 6;
constexpr uint8_t BIT_7_MASK = 0x80;
constexpr uint8_t BIT_0_6_MASK = 0x7f;

class WoSensorTHDataHandler : public SensorDataHandler
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
  const std::string m_device_mac = "C5:63:50:48:45:A2";
};

#endif
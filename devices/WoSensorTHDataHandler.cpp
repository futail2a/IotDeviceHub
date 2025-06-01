#include "WoSensorTHDataHandler.h"
#include "MqttTopicList.h"
#include <iostream>
#include <iomanip>

void WoSensorTHDataHandler::update(std::vector<uint8_t> &data)
{
    m_update_cb(data);
}

void WoSensorTHDataHandler::m_print_sensor_data(const std::vector<uint8_t>& data)
{
//   m_print_byte_array(data);
  if(data.size() > SERVICEDATA_LEN)
  {
    std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    return;
  }

  uint8_t temp = data[4]&BIT_0_6_MASK;
  std::cout << "Temperature: " << ((data[4]&BIT_7_MASK) ? "" : "-") << (int)temp << "°C" << std::endl;

  // Notice: API document explains Bit[7] is Templature Scale and Bit[6:0] is Humidity Value
  // But as far as I checked with actual value from device, whole Bit[7:0] expresse humidity value, and Bit[7] is not temperature scale
  // https://github.com/OpenWonderLabs/SwitchBotAPI-BLE/blob/latest/devicetypes/meter.md#broadcast-mode
  uint8_t humid = data[5];
  std::cout << "Humidity: " << (int)humid << "%" << std::endl;
}

std::vector<MqttMessage> WoSensorTHDataHandler::createPublishMessages(const std::vector<uint8_t>& data)
{
    std::vector<MqttMessage> messages;

    if(data.size() > SERVICEDATA_LEN)
    {
        std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    }
    else
    {
        // TODO: room in topic to be configurable
        std::string temp_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/bed_room" + IOT_TOPIC_SENS_DATA_TEMP;
        uint8_t temp = data[4]&BIT_0_6_MASK;
        std::string signChar = (data[4]&BIT_7_MASK) ? "" : "-";
        std::string temp_str = "Temperature: " + signChar + std::to_string((int)temp) + "°C";
        MqttMessage temp_message{temp_topic_str, temp_str};
        messages.emplace_back(temp_message);

        std::string humid_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/bed_room" + IOT_TOPIC_SENS_DATA_HUMID;
        uint8_t humid = data[5];
        std::string humid_str = "Humidity: " + std::to_string((int)humid) + "%";
        MqttMessage humid_message{humid_topic_str, humid_str};
        messages.emplace_back(humid_message);
    }

    return messages;
}
#include "MotionSensorDataHandler.h"
#include "MqttTopicList.h"
#include <iostream>
#include <iomanip>

constexpr uint8_t SERVICEDATA_LEN = 6;
constexpr uint8_t BIT_7_MASK = 0x80;
constexpr uint8_t BIT_1_0_MASK = 0x03;

void MotionSensorDataHandler::update(std::vector<uint8_t> &data)
{
    m_update_cb(data);
}

void MotionSensorDataHandler::m_print_sensor_data(const std::vector<uint8_t>& data)
{
  if(data.size() > SERVICEDATA_LEN)
  {
    std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    return;
  }

  uint8_t pir_time = data[5]&BIT_7_MASK;
  std::cout << "pir_time: " << std::to_string((int)pir_time) << std::endl;

  uint8_t light_intensity = data[5]&BIT_1_0_MASK;
  std::cout << "light_intensity: " << std::to_string((int)light_intensity) << std::endl;
}

std::vector<MqttMessage> MotionSensorDataHandler::createPublishMessages(const std::vector<uint8_t>& data)
{
    static uint16_t last_pir=0U;
    static uint8_t last_light_intensity=0U;

    std::vector<MqttMessage> messages;

    if(data.size() > SERVICEDATA_LEN)
    {
        std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    }
    else
    {
        // TODO: room in topic to be configurable
        std::string pir_time_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/entrance" + IOT_TOPIC_SENS_DATA_PIR_UTC;
        uint16_t pir_time = (static_cast<uint16_t>(data[3]) << 8) | static_cast<uint16_t>(data[4]);

        // Check if the PIR time has changes
        if(pir_time < last_pir)
        {
            std::string pir_time_str = "Since the last trigger PIR time (s): " + std::to_string(pir_time);
            mosquitto_property* properties = nullptr;
            MqttMessage pir_time_message{pir_time_topic_str, pir_time_str,0,false,properties};
            messages.emplace_back(pir_time_message);
        }
        last_pir = pir_time;


        std::string light_intensity_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/entrance" + IOT_TOPIC_SENS_DATA_LIGHT_INTENSITY;
        uint8_t light_intensity = data[5]&BIT_1_0_MASK;

        // Check if the light intensity has changed
        if(light_intensity != last_light_intensity)
        {
            std::string light_intensity_str;
            if(light_intensity==1U)
            {
              light_intensity_str = "Light Intensity: dark";
            }
            else if(light_intensity==2U)
            {
              light_intensity_str = "Light Intensity: bright";
            }
            else
            {
              light_intensity_str = "Light Intensity: unknown (" + std::to_string(light_intensity) + ")";
              std::cerr << "ERROR: Unexpected light intensity value" << std::endl;
            }
            MqttMessage light_intensity_message{light_intensity_topic_str, light_intensity_str};
            messages.emplace_back(light_intensity_message);
        }
        last_light_intensity = light_intensity;
    }

    return messages;
}

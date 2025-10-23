#include "WoMotionSensorHandler.h"
#include "MqttTopicList.h"
#include <iostream>
#include <iomanip>
#include <filesystem>
#include "Poco/Util/JSONConfiguration.h"

constexpr uint8_t SERVICEDATA_LEN = 6;
constexpr uint8_t BIT_7_MASK = 0x80;
constexpr uint8_t BIT_1_0_MASK = 0x03;

WoMotionSensorHandler::WoMotionSensorHandler()
{
    if(std::filesystem::is_regular_file(CONFIG_FILE_PATH))
    {
        Poco::Util::JSONConfiguration config = Poco::Util::JSONConfiguration(CONFIG_FILE_PATH);
        try
        {
            mDevceMac = config.getString("devices.woMotionSensor.mac");
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    else
    {
        std::cerr << "Config file not found: " << CONFIG_FILE_PATH << std::endl;
    }
}


BleDeviceState WoMotionSensorHandler::getState()
{
  std::lock_guard<std::mutex> lock(mConnStatusMtx);
  return mState;
}
void WoMotionSensorHandler::setState(const BleDeviceState state)
{
    std::lock_guard<std::mutex> lock(mConnStatusMtx);
    mState = state;
}

void WoMotionSensorHandler::onAdvPacketRecived(const std::vector<uint8_t> &data)
{
    if (data.size() < SERVICEDATA_LEN)
    {
        std::cerr << "Received data is too short: " << data.size() << " bytes, expected at least " << SERVICEDATA_LEN << " bytes." << std::endl;
        return;
    }

    static uint16_t last_pir=0U;
    uint16_t pir_time = (static_cast<uint16_t>(data[3]) << 8) | static_cast<uint16_t>(data[4]);

    // Check if the PIR time has changes
    if(pir_time < last_pir)
    {
        std::cout<< "Since the last trigger PIR time (s): " + std::to_string(pir_time) <<std::endl;
        mUpdateCb(data);
    }
    last_pir = pir_time;

    bool someoneMoving = (data[1] & 0x40) != 0;

    if (someoneMoving)
    {
      std::cout << "Someone is moving" << std::endl;
      mUpdateCb(data);
    }
    else
    {
      std::cout << "No one moves\n" << std::endl;;
    }
}

std::vector<MqttMessage> WoMotionSensorHandler::createPublishMessages(const std::vector<uint8_t>& data)
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

void WoMotionSensorHandler::m_print_sensor_data(const std::vector<uint8_t>& data)
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

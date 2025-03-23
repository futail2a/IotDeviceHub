#include "IotDeviceHubManager.h"
#include "BluezAbstructLayer.h"
#include <iostream>
#include <iomanip>

IotDeviceHubManager::IotDeviceHubManager()
{
    //TODO: Create factory method for IotDeviceHubManager to specify IoT device
    m_sensorDataHandler = std::make_shared<WoSensorTHDataHandler>();
    m_bluez =std::make_unique<BluezAbstructLayer>(m_sensorDataHandler);
    m_mqtt =std::make_unique<MqttManager>();
}

void IotDeviceHubManager::stop()
{
    m_bluez->stop_scan();
    m_mqtt->stop();
    m_mqtt->deinit();
}

bool IotDeviceHubManager::init()
{
    if(m_mqtt->init())
    {
      m_mqtt->start();
    }
    else
    {
        std::cerr << "Failed to initialize MQTT" << std::endl;
        return false;
    }

    if(m_bluez->init())
    {
        m_bluez->start_scan();
    }
    else
    {
        std::cerr << "Failed to initialize Bluez" << std::endl;
        return false;
    }

    return true;
}

void IotDeviceHubManager::run()
{
  std::vector<uint8_t> adv_data;

  //Todo: Implement loop logic
  for(int i = 0; i < 5; i++)
  {
    adv_data = m_bluez->get_adv_data();
    if (!adv_data.empty())
    {
        std::vector<MqttMessage> messages = m_sensorDataHandler->createPublishMessages(adv_data);
        for(auto message : messages)
        {
            m_mqtt->publishMessage(message.topic, message.message);
        }
        adv_data.clear();
    }
    sleep(1);
  }


}

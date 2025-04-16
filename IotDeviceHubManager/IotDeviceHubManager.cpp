#include "IotDeviceHubManager.h"
#include "BluezAbstructLayer.h"
#include <iostream>
#include <iomanip>

IotDeviceHubManager::IotDeviceHubManager(){}

bool IotDeviceHubManager::init()
{
    m_bluez =std::make_unique<BluezAbstructLayer>();

    m_th_sensor_data_handler = std::make_shared<WoSensorTHDataHandler>();
    m_th_sensor_data_handler->set_update_cb(std::bind(&IotDeviceHubManager::on_th_update, this, std::placeholders::_1));
    m_bluez->add_sensor_data_handler(m_th_sensor_data_handler);

    m_motion_sensor_data_handler = std::make_shared<MotionSensorDataHandler>();
    m_motion_sensor_data_handler->set_update_cb(std::bind(&IotDeviceHubManager::on_motion_update, this, std::placeholders::_1));
    m_bluez->add_sensor_data_handler(m_motion_sensor_data_handler);

    m_mqtt =std::make_unique<MqttManager>();

    if(m_mqtt->init("iot_device_hub"))
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
  while(true)
  {
    m_bluez->check_adv_data();
    sleep(3);
  }
}

void IotDeviceHubManager::stop()
{
    m_bluez->stop_scan();
    m_mqtt->stop();
    m_mqtt->deinit();
}

void IotDeviceHubManager::on_th_update(std::vector<uint8_t> data)
{
    if (!data.empty())
    {
        std::vector<MqttMessage> messages = m_th_sensor_data_handler->createPublishMessages(data);
        for(auto message : messages)
        {
            m_mqtt->publishMessage(message.topic, message.message);
        }
    }
}

void IotDeviceHubManager::on_motion_update(std::vector<uint8_t> data)
{
    if (!data.empty())
    {
        std::vector<MqttMessage> messages = m_motion_sensor_data_handler->createPublishMessages(data);
        for(auto message : messages)
        {
            m_mqtt->publishMessage(message.topic, message.message);
        }
    }
}
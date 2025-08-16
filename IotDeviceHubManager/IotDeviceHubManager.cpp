#include "IotDeviceHubManager.h"

#include "BleAbstructLayer.h"
#include "BleSockScanManager.h"
#include "BleDbusConnectionManager.h"
#include "WoMotionSensorHandler.h"
#include "WoBulbHandler.h"

#include <iostream>
#include <unistd.h>

IotDeviceHubManager::IotDeviceHubManager()
{}

bool IotDeviceHubManager::init()
{
    mBle =std::make_unique<BleAbstructLayer>();
    if(!mBle->init(std::make_unique<BleSockScanManager>(), std::make_unique<BleDbusConnectionManager>()))
    {
        std::cerr << "Failed to initialize Bluez" << std::endl;
        return false;
    }

    mMotionSensorDevice = std::make_shared<WoMotionSensorHandler>();
    mMotionSensorDevice->setUpdateCb(std::bind(&IotDeviceHubManager::onMotionUpdate, this, std::placeholders::_1));
    mBle->registerScannedDevice(mMotionSensorDevice);

    mBulbDevice = std::make_shared<WoBulbHandler>();
    mBle->registerConnectDevice(mBulbDevice);

    mEventManager = std::make_shared<IotEventManager>();
    mMotionSensorDevice->setMediator(mEventManager);
    mBulbDevice->setMediator(mEventManager);

    // m_mqtt =std::make_unique<MqttManager>();

    // if(m_mqtt->init("iot_device_hub"))
    // {
    //   m_mqtt->start();
    // }
    // else
    // {
    //     std::cerr << "Failed to initialize MQTT" << std::endl;
    //     return false;
    // }

    return true;
}

void IotDeviceHubManager::run()
{
    if(!mBle->start())
    {
        std::cerr << "Failed to start BLE" << std::endl;
        return;
    }

    while(isRunning)
    {
      mBle->connectDevices();
      sleep(1);
    }
}

void IotDeviceHubManager::stop()
{
    isRunning = false;
    mBle->stop();
    // m_mqtt->stop();
    // m_mqtt->deinit();
}

void IotDeviceHubManager::terminate()
{
    if(isRunning)
    {
      stop();
    }
    mBle->terminate();
}

void IotDeviceHubManager::onLightTimeout(Poco::Timer& timer)
{
    std::cout << "Light timer expired, turning off the light" << std::endl;
    auto command = mBulbDevice->getTurnOffCommand();
    mBle->sendBleCommand(command);
    mLightTimer.reset();
}

void IotDeviceHubManager::onMotionUpdate(std::vector<uint8_t> data)
{
  std::cout << "Motion data received" <<std::endl;

  auto command = mBulbDevice->getTurnOnCommand();
  mBle->sendBleCommand(command);

  if(mLightTimer)
  {
    std::cout << "Detected motion again, extend light on timer" <<std::endl;
    mLightTimer->stop();
    mLightTimer->start(Poco::TimerCallback<IotDeviceHubManager>(*this, &IotDeviceHubManager::onLightTimeout));
  }
  else
  {
    mLightTimer = std::make_unique<Poco::Timer>(DEFAULT_LIGHT_INTERVAL, 0);
    mLightTimer->start(Poco::TimerCallback<IotDeviceHubManager>(*this, &IotDeviceHubManager::onLightTimeout));
  }

    // if (!data.empty())
    // {
    //     std::vector<MqttMessage> messages = m_motion_sensor_data_handler->createPublishMessages(data);
    //     // Messages will be empty if the data is not updated, to reduce Publish overhead
    //     if(messages.empty())
    //     {
    //         // std::cerr << "No messages to publish" << std::endl;
    //         return;
    //     }
    //     for(auto message : messages)
    //     {
    //         m_mqtt->publishMessage(message.topic, message.message);
    //     }
    // }
}

// void IotDeviceHubManager::on_th_update(std::vector<uint8_t> data)
// {
//     if (!data.empty())
//     {
//         std::vector<MqttMessage> messages = m_th_sensor_data_handler->createPublishMessages(data);
//         for(auto message : messages)
//         {
//             m_mqtt->publishMessage(message.topic, message.message);
//         }
//     }
// }

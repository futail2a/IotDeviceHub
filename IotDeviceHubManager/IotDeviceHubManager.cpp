#include "IotDeviceHubManager.h"

#include "BleAbstructLayer.h"
#include "BleDbusScanManager.h"
#include "BleDbusConnectionManager.h"
#include "WoMotionSensorHandler.h"
#include "WoBulbHandler.h"

#include <iostream>
#include <unistd.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTime.h>
#include <Poco/Timezone.h>

bool IotDeviceHubManager::init()
{
    mBle =std::make_unique<BleAbstructLayer>();
    if(!mBle->init(std::make_unique<BleDbusScanManager>(), std::make_unique<BleDbusConnectionManager>()))
    {
        std::cerr << "Failed to initialize Bluez" << std::endl;
        return false;
    }

    mBulbDevice = std::make_shared<WoBulbHandler>();
    mBotDevice = std::make_shared<WoHandHandler>();
    mBle->registerConnectDevice(mBulbDevice);
    mBle->registerConnectDevice(mBotDevice);

    mEventManager = std::make_shared<IotEventManager>();
    mBulbDevice->setMediator(mEventManager);

    mMqtt =std::make_unique<MqttManager>();

    if(mMqtt->init("iot_device_hub"))
    {
        mMqtt->subscribe("button");
        mMqtt->subscribe("exec_bot");
        mEventManager->registerEventHandler("exec_bot", [this](const std::string& eventData)
        {
            std::cout << "Event: exec_bot" << std::endl;
            auto command = mBotDevice->getExecActionCommand();
            mBle->sendBleCommand(command);
        }
        );
        mMqtt->setMediator(mEventManager);
        mMqtt->start();
    }
    else
    {
        std::cerr << "Failed to initialize MQTT" << std::endl;
        return false;
    }

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
        sleep(5);
    }
}

void IotDeviceHubManager::stop()
{
    isRunning = false;
    mBle->stop();
    mMqtt->stop();
    mMqtt->deinit();
}

void IotDeviceHubManager::terminate()
{
    if(isRunning)
    {
      stop();
    }
    mBle->terminate();
}

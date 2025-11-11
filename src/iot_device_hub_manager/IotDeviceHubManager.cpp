#include "IotDeviceHubManager.h"

#include "BleAbstructLayer.h"
#include "BleDbusScanManager.h"
#include "BleDbusConnectionManager.h"
#include "WoMotionSensorHandler.h"
#include "WoBulbHandler.h"
#include "MqttManager.h"
#include "IotEventManager.h"

#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <Poco/LocalDateTime.h>
#include <Poco/DateTime.h>
#include <Poco/Timezone.h>
#include "Poco/Util/JSONConfiguration.h"

const std::string CONFIG_FILE_PATH = "config/config.json";

bool IotDeviceHubManager::init()
{
    mBle =std::make_unique<BleAbstructLayer>();
    if(!mBle->init(std::make_unique<BleDbusScanManager>(), std::make_unique<BleDbusConnectionManager>()))
    {
        std::cerr << "Failed to initialize Bluez" << std::endl;
        return false;
    }

    mEventManager = std::make_shared<IotEventManager>();

    getConfigurationParameters();
    if(mBulbDevice)
    {
        mBle->registerConnectDevice(mBulbDevice);
        mBulbDevice->setMediator(mEventManager);
    }

    if(mBotDevice)
    {
        mBle->registerConnectDevice(mBotDevice);
    }

    if(mMotionSensorDevice)
    {
        mBle->registerScannedDevice(mMotionSensorDevice);
        mMotionSensorDevice->setUpdateCb([this](std::vector<uint8_t> data){ this->onMotionUpdate(data); });
    }

    if(mMqtt && mMqtt->init("iot_device_hub"))
    {
        mMqtt->subscribe("exec_bot");
        mMqtt->subscribe("switch_bulb");
        mEventManager->registerEventHandler("exec_bot", [this](const std::string& eventData)
        {
            std::cout << "Event: exec_bot" << std::endl;
            auto command = mBotDevice->getExecActionCommand();
            mBle->sendBleCommand(command);
        }
        );
        mEventManager->registerEventHandler("switch_bulb", [this](const std::string& eventData)
        {
            std::cout << "Event: switch_bulb" << std::endl;
            BleCommand command{};
            if(eventData == "ON")
            {
                command = mBulbDevice->getTurnOnCommand();
                mBle->sendBleCommand(command);
            }
            else if(eventData == "OFF")
            {
                command = mBulbDevice->getTurnOffCommand();
                mBle->sendBleCommand(command);
            }
            else
            {
                std::cerr << "Unknown bulb command: " << eventData << std::endl;
                return;
            }
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

void IotDeviceHubManager::getConfigurationParameters()
{
    if(std::filesystem::is_regular_file(CONFIG_FILE_PATH))
    {
        Poco::Util::JSONConfiguration config = Poco::Util::JSONConfiguration(CONFIG_FILE_PATH);
        try
        {
            std::string mac = config.getString("devices.woBulb.mac");
            if(!mac.empty())
            {
                mBulbDevice = std::make_shared<WoBulbHandler>(mac);
            }
            else
            {
                std::cout << "woBulb device not configured" << std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            std::string mac= config.getString("devices.woHand.mac");
            if(!mac.empty())
            {
                mBotDevice = std::make_shared<WoHandHandler>(mac);
            }
            else
            {
                std::cout << "woHand device not configured" << std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            std::string mac = config.getString("devices.woMotionSensor.mac");
            if(!mac.empty())
            {
                mMotionSensorDevice = std::make_shared<WoMotionSensorHandler>(mac);
            }
            else
            {
                std::cout << "WoMotionSensorHandler device not configured" << std::endl;
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        try
        {
            std::string brokerIpv4 = config.getString("mqtt.brokerIpv4");
            if(brokerIpv4.empty())
            {
                throw std::runtime_error("MQTT broker IP is empty");
            }

            std::string brokerPort = config.getString("mqtt.brokerPort");
            if(brokerPort.empty())
            {
                throw std::runtime_error("MQTT broker port is empty");
            }

            std::string caCertPath = config.getString("mqtt.caCert");
            std::string clientCertPath = config.getString("mqtt.clientCert");
            std::string clientKeyPath = config.getString("mqtt.clientKey");

            mMqtt =std::make_unique<MqttManager>(brokerIpv4, static_cast<std::uint16_t>(std::stoi(brokerPort)), caCertPath, clientCertPath, clientKeyPath);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    else
    {
        std::cerr << "Config file not found: " << CONFIG_FILE_PATH << ", initialize with default value" << std::endl;

        const std::string DEFAULT_MAC = "00:00:00:00:00:00";
        mBulbDevice = std::make_shared<WoBulbHandler>(DEFAULT_MAC);
        mBotDevice = std::make_shared<WoHandHandler>(DEFAULT_MAC);
        mMotionSensorDevice = std::make_shared<WoMotionSensorHandler>(DEFAULT_MAC);

        const std::string brokerIpv4 = "127.0.0.1";
        const std::string caCertPath = "";
        const std::string clientCertPath = "";
        const std::string clientKeyPath = "";
        mMqtt =std::make_unique<MqttManager>(brokerIpv4, 1883U, caCertPath, clientCertPath, clientKeyPath);
    }

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

}

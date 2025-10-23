#include "WoBulbHandler.h"
#include <iostream>
#include <filesystem>
#include "Poco/Util/JSONConfiguration.h"

WoBulbHandler::WoBulbHandler()
{
    if(std::filesystem::is_regular_file(CONFIG_FILE_PATH))
    {
        Poco::Util::JSONConfiguration config = Poco::Util::JSONConfiguration(CONFIG_FILE_PATH);
        try
        {
            mDevceMac = config.getString("devices.woBulb.mac");
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

BleDeviceState WoBulbHandler::getState()
{
  std::lock_guard<std::mutex> lock(mConnStatusMtx);
  return mState;
}

void WoBulbHandler::setState(const BleDeviceState state)
{
    std::lock_guard<std::mutex> lock(mConnStatusMtx);
    mState = state;
}

void WoBulbHandler::onAdvPacketRecived(const std::vector<uint8_t> &data)
{
    // Nothing to do
}

void WoBulbHandler::onConnected()
{
    std::cout << "Color Bulb is connected" << std::endl;
}

void WoBulbHandler::onDisconnected()
{    std::cout << "Color Bulb is disconnected" << std::endl;
}

void WoBulbHandler::subscribeEvent()
{
  if(mMediator)
    {
        mMediator->registerEventHandler("SomeoneDetected", [this](const std::string& eventData)
        {
            std::cout << "Event: SomeoneDetected" << std::endl;
        }
        );
    }
}

BleCommand WoBulbHandler::getTurnOnCommand() const
{
    BleCommand command;
    command.macAddr = mDevceMac;
    command.charPath = mTurnOffOnCharacteristicPath;
    command.method = "WriteValue";
    command.data = mTurnOnBytes;
    command.options = {{"type", "command"}};
    return command;
}

BleCommand WoBulbHandler::getTurnOffCommand() const
{
    BleCommand command;
    command.macAddr = mDevceMac;
    command.charPath = mTurnOffOnCharacteristicPath;
    command.method = "WriteValue";
    command.data = mTurnOffBytes;
    command.options = {{"type", "command"}};
    return command;
}
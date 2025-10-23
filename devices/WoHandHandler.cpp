#include "WoHandHandler.h"
#include <iostream>
#include <filesystem>
#include "Poco/Util/JSONConfiguration.h"

WoHandHandler::WoHandHandler()
{
    if(std::filesystem::is_regular_file(CONFIG_FILE_PATH))
    {
        Poco::Util::JSONConfiguration config = Poco::Util::JSONConfiguration(CONFIG_FILE_PATH);
        try
        {
            mDevceMac = config.getString("devices.woHand.mac");
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

BleDeviceState WoHandHandler::getState()
{
  std::lock_guard<std::mutex> lock(mConnStatusMtx);
  return mState;
}

void WoHandHandler::setState(const BleDeviceState state)
{
    std::lock_guard<std::mutex> lock(mConnStatusMtx);
    mState = state;
}

void WoHandHandler::onAdvPacketRecived(const std::vector<uint8_t> &data)
{
    // Nothing to do
}

void WoHandHandler::onConnected()
{
    std::cout << "Bot is connected" << std::endl;
}

void WoHandHandler::onDisconnected()
{    std::cout << "Bot is disconnected" << std::endl;
}

void WoHandHandler::subscribeEvent()
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

BleCommand WoHandHandler::getExecActionCommand() const
{
    BleCommand command;
    command.macAddr = mDevceMac;
    command.charPath = mExecActionPath;
    command.method = "WriteValue";
    command.data = mExecActionBytes;
    command.options = {{"type", "command"}};
    return command;
}

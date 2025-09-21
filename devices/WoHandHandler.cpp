#include "WoHandHandler.h"
#include <iostream>

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

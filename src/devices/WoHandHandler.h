#ifndef WO_HAND_HANDLER_H
#define WO_HAND_HANDLER_H

#include "BleDeviceHandler.h"
#include <mutex>

class WoHandHandler : public BleDeviceHandler
{
public:
    WoHandHandler(const std::string mac) : mDevceMac(mac) {};
    ~WoHandHandler() = default;

    std::string getMacAddr() const override  { return mDevceMac; };
    BleDeviceState getState() override;
    void setState(const BleDeviceState state) override;
    void setMediator(std::shared_ptr<IIotEventManager> manager) override { mMediator = manager; }

    void onAdvPacketRecived(const std::vector<uint8_t> &data) override;
    void onConnected() override;
    void onDisconnected() override;
    void subscribeEvent() override {};// nothing to do

    BleCommand getExecActionCommand() const;

private:
    std::string mDevceMac = "";
    const std::string mExecActionPath = "/service0011/char0015"; // org/bluez/hci0/dev_C1_56_82_D9_A9_DA/service0011/char0015
    const std::vector<uint8_t> mExecActionBytes  {0x57, 0x01, 0x00};
    BleDeviceState mState = BleDeviceState::DISCONNECTED;
    std::mutex mConnStatusMtx;
    std::shared_ptr<IIotEventManager> mMediator;
};

#endif
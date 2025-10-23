#ifndef WO_BULBHANDLER_H
#define WO_BULBHANDLER_H

#include "BleDeviceHandler.h"
#include <mutex>

class WoBulbHandler : public BleDeviceHandler
{
public:
    WoBulbHandler();
    ~WoBulbHandler() = default;

    std::string getMacAddr() const override  { return mDevceMac; };
    BleDeviceState getState() override;
    void setState(const BleDeviceState state) override;
    void setMediator(std::shared_ptr<IotEventManager> manager) override { mMediator = manager; }

    void onAdvPacketRecived(const std::vector<uint8_t> &data) override;
    void onConnected() override;
    void onDisconnected() override;
    void subscribeEvent() override;

    BleCommand getTurnOnCommand() const;
    BleCommand getTurnOffCommand() const;

private:
    std::string mDevceMac = "";
    const std::string mTurnOffOnCharacteristicPath = "/service0028/char002c";
    const std::vector<uint8_t> mTurnOnBytes  {0x57, 0x0F, 0x47, 0x01, 0x01};
    const std::vector<uint8_t> mTurnOffBytes {0x57, 0x0F, 0x47, 0x01, 0x02};
    BleDeviceState mState = BleDeviceState::DISCONNECTED;
    std::mutex mConnStatusMtx;
    std::shared_ptr<IotEventManager> mMediator;
};

#endif
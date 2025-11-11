#ifndef WO_MOTION_SENSOR_HANDLER_H
#define WO_MOTION_SENSOR_HANDLER_H

#include "BleDeviceHandler.h"
#include "IIotEventManager.h"

#include <string>
#include <vector>
#include <mosquitto.h>
#include <mutex>
#include <memory>

using UpdateCb = std::function <void(std::vector<uint8_t>)>;

// Todo, separeate this to a different file
struct MqttMessage
{
    std::string topic;
    std::string message;
    int qos=0;
    bool retain=false;
    mosquitto_property* properties=nullptr;
};

class WoMotionSensorHandler : public BleDeviceHandler
{
public:
    WoMotionSensorHandler(const std::string mac): mDevceMac(mac) {};
    ~WoMotionSensorHandler() = default;

    std::string getMacAddr() const override { return mDevceMac; };
    BleDeviceState getState() override;
    void setState(const BleDeviceState state) override;
    void setMediator(std::shared_ptr<IIotEventManager> manager) override { mMediator = manager; }

    void onAdvPacketRecived(const std::vector<uint8_t> &data) override;
    void onConnected() override;
    void onDisconnected() override;
    void subscribeEvent() override {};// nothing to do

    void setUpdateCb(UpdateCb cb) { mUpdateCb = cb; }
    std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data);

private:
  void m_print_sensor_data(const std::vector<uint8_t>& data);
  UpdateCb mUpdateCb = nullptr;
  BleDeviceState mState = BleDeviceState::DISCONNECTED;
  std::mutex mConnStatusMtx;

  std::string mDevceMac = "";
  std::shared_ptr<IIotEventManager> mMediator;
};

#endif
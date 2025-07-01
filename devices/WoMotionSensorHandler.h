#ifndef WO_MOTION_SENSOR_HANDLER_H
#define WO_MOTION_SENSOR_HANDLER_H

#include "BleDeviceHandler.h"
#include "IotEventManager.h"

#include <string>
#include <vector>
#include <mosquitto.h>
#include <mutex>
#include <memory>

// Todo, separeate this to a different file
struct MqttMessage
{
    std::string topic;
    std::string message;
    int qos=0;
    bool retain=false;
    mosquitto_property* properties=nullptr;
};

using UpdateCb = std::function <void(std::vector<uint8_t>)>;

class WoMotionSensorHandler : public BleDeviceHandler
{
public:
    WoMotionSensorHandler() = default;
    ~WoMotionSensorHandler() = default;

    std::string getMacAddr() const override { return mDevceMac; };
    BleDeviceState getState() override;
    void setState(const BleDeviceState state) override;
    void setMediator(std::shared_ptr<IotEventManager> manager) override { mMediator = manager; }

    void onAdvPacketRecived(const std::vector<uint8_t> &data) override;
    void onConnected() override {};// nothing to do
    void onDisconnected() override {};// nothing to do
    void subscribeEvent() override {};// nothing to do

    void setUpdateCb(UpdateCb cb) { mUpdateCb = cb; }
    std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data);

private:
  void m_print_sensor_data(const std::vector<uint8_t>& data);
  UpdateCb mUpdateCb;
  BleDeviceState mState = BleDeviceState::DISCONNECTED;
  std::mutex mConnStatusMtx;

  // Hardcord the MAC address of the device to be found
  // TODO: To find target adv data by service UUID
  const std::string mDevceMac = "B0:E9:FE:55:04:12";
  std::shared_ptr<IotEventManager> mMediator;
};

#endif
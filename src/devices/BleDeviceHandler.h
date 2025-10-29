#ifndef BLE_DEVICE_HANDLER_H
#define BLE_DEVICE_HANDLER_H

#include "IotEventManager.h"

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <cstdint>

using AdPacketReceivedCallback = std::function<void(std::vector<uint8_t>)>;
using ConnectCallback = std::function<void(std::vector<uint8_t>)>;

enum class BleDeviceState
{
    DISCONNECTED = 0,
    CONNECTING,
    CONNECTED,
    DISCONNECTING
};

struct BleCommand
{
    std::string macAddr; // MAC address of the device
    std::string charPath; //characteristic path
    std::string method; // "ReadValue", "WriteValue"
    std::vector<uint8_t> data; // data to write
    std::map<std::string, std::string> options;
    std::uint16_t timeout = 1000; // ms
};

class BleDeviceHandler
{
public:
    BleDeviceHandler() = default;
    ~BleDeviceHandler() = default;

    virtual std::string getMacAddr() const = 0;
    virtual BleDeviceState getState() = 0;
    virtual void setState(const BleDeviceState state) = 0;
    virtual void setMediator(std::shared_ptr<IotEventManager> manager) = 0;

    virtual void onAdvPacketRecived(const std::vector<uint8_t> &data) = 0;
    virtual void onConnected() = 0;
    virtual void onDisconnected() = 0;
    virtual void subscribeEvent() = 0;
};

#endif
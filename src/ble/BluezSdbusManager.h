#ifndef BLUEZ_SDBUS_MANAGER_H
#define BLUEZ_SDBUS_MANAGER_H

#include "IBleManager.h"

#include <sdbus-c++/sdbus-c++.h>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <thread>
#include <atomic>

const std::string BLUEZ_SERVICE = "org.bluez";
const std::string BLUEZ_PATH = "/org/bluez";
const std::string HCI0_PATH = "/org/bluez/hci0";
const std::string BLUEZ_ADAPTER = "org.bluez.Adapter1";
const std::string BLUEZ_DEVICE = "org.bluez.Device1";
const std::string GATT_CHAR_1 = "org.bluez.GattCharacteristic1";
const std::string METHOD_CONNECT = "Connect";

inline std::string convertToBluezMac(const std::string addr)
{
    std::string out;
    for (char c : addr)
    {
        if (c != ':')
        {
            out += std::tolower(static_cast<unsigned char>(c));
        }
        else
        {
            out += "_";
        }
    }
    return out;
}

// "/org/bluez/hci0/dev_xx_xx_xx" → "XX:XX:XX:XX:XX:XX"
inline std::string convertFromBluezMac(const std::string addr)
{
    std::string out;
    auto mac = addr.substr(addr.find("dev_") + 4);
    for (char c : mac)
    {
        if (c != '_')
        {
            out += std::toupper(static_cast<unsigned char>(c));
        }
        else
        {
            out += ":";
        }
    }
    return out;
}

class BluezSdbusManager : public IBleManager
{
public:
    BluezSdbusManager() = default;
    ~BluezSdbusManager() = default;
    bool init() override;
    void terminate() override;
    bool startScan() override;
    bool stopScan() override;
    void setDevice(std::shared_ptr<BleDeviceHandler> device) override;
    void connectDevices() override;
    void sendCommand(const BleCommand& command) override;
    std::weak_ptr<BleDeviceHandler> findDeviceFromMac(std::string mac);

    static void onPropertiesChanged(sdbus::Signal& signal, BluezSdbusManager* handler);

private:
    void onInterfacesAdded(sdbus::Signal& signal);
    std::unique_ptr<sdbus::IConnection> mConnection;
    std::unique_ptr<sdbus::IProxy> mBluezProxy;
    std::vector<std::weak_ptr<BleDeviceHandler>> mBleDeviceHandlers;
};

#endif
#ifndef BLE_DBUS_SCAN_MANAGER_H
#define BLE_DBUS_SCAN_MANAGER_H

#include "BleScanManager.h"
#include "BleDeviceHandler.h"
#include "BleDbusConnectionManager.h"

#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <dbus/dbus.h>

constexpr uint8_t MAC_LENGTH = 6;
constexpr uint8_t SERVICE_DATA_TYPE = 0x16;
constexpr uint8_t SERVICE_DATA_INDEX = 4;
constexpr long SOCK_TIMEOUT_USEC = 100000; // 100 ms

// const std::string BLUEZ_PATH = "/org/bluez";
// const std::string ADAPTER_PATH = "/org/bluez/hci0";
// const std::string BLUEZ_ADAPTER = "org.bluez.Adapter1";
// const std::string BLUEZ_DEVICE = "org.bluez.Device1";
// const std::string GATT_CHAR_1 = "org.bluez.GattCharacteristic1";
// const std::string BLUEZ_SERVICE = "org.bluez";
// // const std::string DBUS_PROPERTIES = "org.freedesktop.DBus.Properties";
// const std::string METHOD_CONNECT = "Connect";

class BleDbusScanManager : public BleScanManager
{
public:
    BleDbusScanManager() = default;
    ~BleDbusScanManager() = default;

    bool init() override;
    void terminate() override;
    bool startScan() override;
    bool stopScan() override;
    void setDevice(std::shared_ptr<BleDeviceHandler> deviceHandler) override;

    void scanning();

private:
    std::vector<std::pair<std::shared_ptr<BleDeviceHandler>,std::string>> mBleDeviceHandlerMacPairs; // pari of handler and converted mac address
    std::unique_ptr<DBusConnection, DBusConnectionDeleter> mConn;

    void dbusLoop();
    std::atomic<bool> isRunning{false};
    std::thread mDbusThread;
};

#endif
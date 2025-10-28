#ifndef BLE_DBUS_CONNECTION_MANAGER_H
#define BLE_DBUS_CONNECTION_MANAGER_H

#include "BleConnectionManager.h"
#include "BleDeviceHandler.h"

#include <vector>
#include <memory>
#include <dbus/dbus.h>
#include <atomic>
#include <thread>

const std::string BLUEZ_PATH = "/org/bluez";
const std::string ADAPTER_PATH = "/org/bluez/hci0";
const std::string BLUEZ_ADAPTER = "org.bluez.Adapter1";
const std::string BLUEZ_DEVICE = "org.bluez.Device1";
const std::string GATT_CHAR_1 = "org.bluez.GattCharacteristic1";
const std::string BLUEZ_SERVICE = "org.bluez";
const std::string METHOD_CONNECT = "Connect";

struct DBusConnectionDeleter
{
    void operator()(DBusConnection* conn) const
    {
        if (conn)
        {
            dbus_connection_unref(conn);
        }
    }
};


class BleDbusConnectionManager : public BleConnectionManager
{
public:
    BleDbusConnectionManager() = default;
    ~BleDbusConnectionManager();

    bool init() override;
    void terminate() override;
    void setDevice(std::shared_ptr<BleDeviceHandler> device) override;
    void connect() override;
    void sendCommand(const BleCommand& command) override;

    static void connectionCb(DBusPendingCall* pending, void* user_data);
    static DBusHandlerResult propertiesChangedCb(DBusConnection* conn, DBusMessage* msg, void* user_data);

private:
    std::vector<std::pair<std::shared_ptr<BleDeviceHandler>,std::string>> mBleDeviceHandlerMacPairs; // pari of handler and converted mac address
    std::unique_ptr<DBusConnection, DBusConnectionDeleter> mConn;

    void dbusLoop();
    std::atomic<bool> isRunning{false};
    std::thread mDbusThread;
};

#endif
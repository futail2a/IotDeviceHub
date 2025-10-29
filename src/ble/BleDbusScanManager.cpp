#include "BleDbusScanManager.h"
#include <iostream>

bool BleDbusScanManager::init()
{
    DBusError err;
    dbus_error_init(&err);
    mConn = std::unique_ptr<DBusConnection, DBusConnectionDeleter>(dbus_bus_get(DBUS_BUS_SYSTEM, &err), DBusConnectionDeleter());
    if (!mConn)
    {
        std::cerr << "Failed to connect to DBus: " << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    if (dbus_error_is_set(&err))
    {
        std::cerr << "DBus Connection Error: " << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    isRunning = true;
    mDbusThread = std::thread(&BleDbusScanManager::dbusLoop, this);
    return true;
}

void BleDbusScanManager::terminate()
{
    isRunning = false;
    if (mDbusThread.joinable())
    {
        mDbusThread.join();
    }

    if (!mConn)
    {
        mConn.reset();
    }
}

void BleDbusScanManager::dbusLoop()
{
    if (!mConn)
    {
        std::cerr << "DBus connection not initialized" << std::endl;
        return;
    }

    while (isRunning)
    {
        dbus_connection_read_write_dispatch(mConn.get(), 100);
    }
}

void BleDbusScanManager::setDevice(std::shared_ptr<BleDeviceHandler> device)
{
    if (device)
    {
        std::string devicePath = ADAPTER_PATH + "/dev_" + device->getMacAddr();
        std::replace(devicePath.begin(), devicePath.end(), ':', '_');
        mBleDeviceHandlerMacPairs.push_back({device, devicePath});
    }
    else
    {
        std::cerr << "Sensor data handler is null" << std::endl;
    }
}


bool BleDbusScanManager::startScan()
{
    if (!mConn)
    {
        std::cerr << "DBus connection not initialized" << std::endl;
        return false;
    }

    DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), ADAPTER_PATH.c_str(),
                                                    BLUEZ_ADAPTER.c_str(), "StartDiscovery");
    if (!msg)
    {
        std::cerr << "Failed to create DBus message for StartDiscovery" << std::endl;
        return false;
    }

    DBusError err;
    dbus_error_init(&err);
    DBusPendingCall* pending;
    if (!dbus_connection_send_with_reply(mConn.get(), msg, &pending, -1))
    {
        std::cerr << "Failed to send StartDiscovery message: " << err.message << std::endl;
        dbus_error_free(&err);
        dbus_message_unref(msg);
        return false;
    }

    dbus_message_unref(msg);

    if (!pending)
    {
        std::cerr << "Pending call is null" << std::endl;
        return false;
    }

    return true;
}

bool BleDbusScanManager::stopScan()
{
    if (!mConn)
    {
        std::cerr << "DBus connection not initialized" << std::endl;
        return false;
    }

    DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), ADAPTER_PATH.c_str(),
                                                    BLUEZ_ADAPTER.c_str(), "StopDiscovery");
    if (!msg)
    {
        std::cerr << "Failed to create DBus message for StopDiscovery" << std::endl;
        return false;
    }

    DBusError err;
    dbus_error_init(&err);
    if (!dbus_connection_send_with_reply(mConn.get(), msg, nullptr, -1))
    {
        std::cerr << "Failed to send StopDiscovery message: " << err.message << std::endl;
        dbus_error_free(&err);
        dbus_message_unref(msg);
        return false;
    }

    dbus_message_unref(msg);
    std::cout << "Scan stopped" << std::endl;

    return true;
}
#include "BleDbusConnectionManager.h"
#include <iostream>

BleDbusConnectionManager::~BleDbusConnectionManager()
{
    isRunning = false;
    if (mDbusThread.joinable())
    {
        mDbusThread.join();
    }
}

bool BleDbusConnectionManager::init()
{
    DBusError err;
    dbus_error_init(&err);
    mConn = std::unique_ptr<DBusConnection, DBusConnectionDeleter>(dbus_bus_get(DBUS_BUS_SYSTEM, &err), DBusConnectionDeleter());
    if (!mConn) {
        std::cerr << "Failed to connect to DBus: " << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    if (dbus_error_is_set(&err)) {
        std::cerr << "DBus Connection Error: " << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    isRunning = true;
    mDbusThread = std::thread(&BleDbusConnectionManager::dbusLoop, this);
    return true;
}

void BleDbusConnectionManager::terminate()
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

void BleDbusConnectionManager::dbusLoop()
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

void BleDbusConnectionManager::setDevice(std::shared_ptr<BleDeviceHandler> device)
{
    if (device)
    {
        std::string devicePath = ADAPTER_PATH + "/dev_" + device->getMacAddr();
        std::replace(devicePath.begin(), devicePath.end(), ':', '_');
        mBleDeviceHandlerMacPairs.push_back({device, devicePath});

        // Subscribe connection status
        if(mConn)
        {
            std::string matchRule = "type='signal',interface='org.freedesktop.DBus.Properties',"
                                    "member='PropertiesChanged',path='" + devicePath + "'";
            dbus_bus_add_match(mConn.get(), matchRule.c_str(), nullptr);
            dbus_connection_add_filter(mConn.get(), &BleDbusConnectionManager::propertiesChangedCb, new std::shared_ptr<BleDeviceHandler>(device),
                [](void* data) { delete static_cast<std::shared_ptr<BleDeviceHandler>*>(data); });
            std::cout << "Subscribed to properties changed for device: " << device->getMacAddr() << std::endl;
        }
        else
        {
            std::cerr << "DBus connection is not initialized" << std::endl;
        }
    }
    else
    {
        std::cerr << "Sensor data handler is null" << std::endl;
    }
}

void BleDbusConnectionManager::connectionCb(DBusPendingCall* pending, void* user_data)
{
    auto handler = static_cast<std::shared_ptr<BleDeviceHandler>*>(user_data);
    DBusMessage* reply = dbus_pending_call_steal_reply(pending);
    if (reply) {
        if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN)
        {
            std::cout << "Connection request to " <<  (*handler)->getMacAddr() << " succeeded" << std::endl;
            (*handler)->setState(BleDeviceState::CONNECTED);
            (*handler)->onConnected();
        }
        else
        {
            std::cerr << "Failed to connect to " <<  (*handler)->getMacAddr() << ": " << dbus_message_get_error_name(reply) << std::endl;
            (*handler)->setState(BleDeviceState::DISCONNECTED);
            (*handler)->onDisconnected();
        }
        dbus_message_unref(reply);
    }
    else
    {
        std::cerr << "Failed to get reply for connection attempt" << std::endl;
        (*handler)->setState(BleDeviceState::DISCONNECTED);
        (*handler)->onDisconnected();
    }
    dbus_pending_call_unref(pending);
}

void BleDbusConnectionManager::connect()
{
    if (mBleDeviceHandlerMacPairs.empty()) {
        std::cout << "No BLE devices registered for connection" << std::endl;
        return;
    }
    if (!mConn)
    {
        std::cerr << "DBus connection not initialized" << std::endl;
        return;
    }

    for (const auto& handler : mBleDeviceHandlerMacPairs)
    {
        if(handler.first->getState() != BleDeviceState::DISCONNECTED)
        {
            // std::cout << "Device " << handler.first->getMacAddr() << " is not in DISCONNECTED state: " << static_cast<int>(handler.first->getState()) << ", skipping connection attempt." << std::endl;
            continue;
        }

        DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), handler.second.c_str(), BLUEZ_DEVICE.c_str(), METHOD_CONNECT.c_str());
        if (!msg)
        {
            std::cerr << "Failed to create DBus message\n";
            return;
        }

        dbus_message_append_args(msg, DBUS_TYPE_INVALID);

        // Ref. Blocking request for connection
        // DBusError err;
        // dbus_error_init(&err);
        // DBusMessage* reply = dbus_connection_send_with_reply_and_block(mConn.get(), msg, 5000, &err);
        // if (dbus_error_is_set(&err))
        // {
        //     std::cerr << "Connection Error: " << err.message << std::endl;
        //     dbus_error_free(&err);
        //       return;
        // }

        // if (reply)
        // {
        //     if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_METHOD_RETURN)
        //     {
        //         std::cout << "Blocking connect: METHOD_RETURN received for " << handler.first->getMacAddr() << std::endl;
        //         handler.first->setState(BleDeviceState::CONNECTED);
        //         handler.first->onConnected();
        //     }
        //     else if (dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR)
        //     {
        //         std::cerr << "Blocking connect: ERROR received for " << handler.first->getMacAddr() << ", error name: " << dbus_message_get_error_name(reply) << std::endl;
        //         handler.first->setState(BleDeviceState::DISCONNECTED);
        //     }
        //     dbus_message_unref(reply);
        // }
        // else
        // {
        //     std::cerr << "Blocking connect: No reply received for connection attempt to " << handler.second << std::endl;
        //     handler.first->setState(BleDeviceState::DISCONNECTED);
        // }
        // dbus_message_unref(msg);

        DBusPendingCall* pending = nullptr;
        if (!dbus_connection_send_with_reply(mConn.get(), msg, &pending, -1))
        {
            std::cerr << "Failed to send connection message" << std::endl;
            dbus_message_unref(msg);
            return;
        }
        if (!pending)
        {
            std::cerr << "Pending call is null" << std::endl;
            dbus_message_unref(msg);
            return;
        }
        else
        {
            std::cout << "Connection request sent for " << handler.first->getMacAddr() << std::endl;
            handler.first->setState(BleDeviceState::CONNECTING);
        }

        dbus_pending_call_set_notify(pending, &BleDbusConnectionManager::connectionCb, new std::shared_ptr<BleDeviceHandler>(handler.first),
            [](void* data) { delete static_cast<std::shared_ptr<BleDeviceHandler>*>(data); });
        dbus_message_unref(msg);
        // dbus_connection_read_write_dispatch(mConn.get(), -1);

        continue;
    }
}

DBusHandlerResult BleDbusConnectionManager::propertiesChangedCb(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    std::cout << "PropertiesChanged signal received" << std::endl;
    if (dbus_message_is_signal(msg, "org.freedesktop.DBus.Properties", "PropertiesChanged"))
    {
        DBusMessageIter args;
        dbus_message_iter_init(msg, &args);

        const char* interface;
        dbus_message_iter_get_basic(&args, &interface);

        if (std::string(interface) != "org.bluez.Device1")
        {
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        dbus_message_iter_next(&args); // move to changed properties
        DBusMessageIter dictIter;
        dbus_message_iter_recurse(&args, &dictIter);

        while (dbus_message_iter_get_arg_type(&dictIter) == DBUS_TYPE_DICT_ENTRY)
        {
            DBusMessageIter entryIter;
            dbus_message_iter_recurse(&dictIter, &entryIter);

            const char* key;
            dbus_message_iter_get_basic(&entryIter, &key);

            if (std::string(key) == "Connected")
            {
                dbus_message_iter_next(&entryIter);
                DBusMessageIter variantIter;
                dbus_message_iter_recurse(&entryIter, &variantIter);

                dbus_bool_t connected;
                dbus_message_iter_get_basic(&variantIter, &connected);

                auto handler = static_cast<std::shared_ptr<BleDeviceHandler>*>(user_data);
                // auto* handler = static_cast<BleDeviceHandler*>(user_data);
                if (connected)
                {
                    std::cout << "Device " << (*handler)->getMacAddr() << " is connected" << std::endl;
                    (*handler)->setState(BleDeviceState::CONNECTED);
                    (*handler)->onConnected();
                }
                else
                {
                    std::cout << "Device " << (*handler)->getMacAddr() << " is disconnected" << std::endl;
                    (*handler)->setState(BleDeviceState::DISCONNECTED);
                    (*handler)->onDisconnected();
                }
            }

            dbus_message_iter_next(&dictIter);
        }

        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


void BleDbusConnectionManager::sendCommand(const BleCommand& command)
{
    if (!mConn)
    {
        std::cerr << "DBus connection not initialized" << std::endl;
        return;
    }

    auto it = std::find_if(mBleDeviceHandlerMacPairs.begin(), mBleDeviceHandlerMacPairs.end(),
                           [&command](const auto& pair) { return pair.first->getMacAddr() == command.macAddr; });
    if (it == mBleDeviceHandlerMacPairs.end())
    {
        std::cerr << "Device with MAC " << command.macAddr << " not found" << std::endl;
        return;

    }

    if (it->first->getState() != BleDeviceState::CONNECTED)
    {
        std::cout << "Device " << it->first->getMacAddr() << " is not in CONNECTED state: "
                    << static_cast<int>(it->first->getState()) << ", skipping command send." << std::endl;
        return;
    }

    std::string fullPath = it->second + command.charPath;
    std::cout << "Sending command to " << it->first->getMacAddr() << " at path: " << fullPath << std::endl;
    DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), fullPath.c_str(),
                                                    GATT_CHAR_1.c_str(), command.method.c_str());
    if (!msg)
    {
        std::cerr << "Failed to create DBus message for command" << std::endl;
        return;
    }

    DBusMessageIter args;
    dbus_message_iter_init_append(msg, &args);

    DBusMessageIter arrayIter;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "y", &arrayIter);
    for (uint8_t b : command.data) {
        dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_BYTE, &b);
    }
    dbus_message_iter_close_container(&args, &arrayIter);

    DBusMessageIter dictIter;
    dbus_message_iter_open_container(&args, DBUS_TYPE_ARRAY, "{sv}", &dictIter);
    dbus_message_iter_close_container(&args, &dictIter);

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(mConn.get(), msg, 5000, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err))
    {
        std::cerr << "Error sending command: " << err.message << std::endl;
        dbus_error_free(&err);
        return;
    }

    if (reply)
    {
        dbus_message_unref(reply);
        std::cout << "Command sent successfully to " << it->first->getMacAddr() << std::endl;
    }
    else
    {
        std::cerr << "No reply received for command sent to " << it->first->getMacAddr() << std::endl;
    }
}

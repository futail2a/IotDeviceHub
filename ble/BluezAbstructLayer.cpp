#include "BluezAbstructLayer.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>

BluezAbstructLayer::BluezAbstructLayer()
{
    m_conn = nullptr;
    m_adapter_path = BLUEZ_PATH + "/hci0";
}

BluezAbstructLayer::~BluezAbstructLayer()
{
    if (m_conn) {
        dbus_connection_unref(m_conn);
    }
}

bool BluezAbstructLayer::init()
{
    DBusError err;
    dbus_error_init(&err);

    m_conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (!m_conn) {
        std::cerr << "Failed to connect to DBus: " << err.message << std::endl;
        dbus_error_free(&err);
        return false;
    }

    createDbusMessages();
    return true;
}

void BluezAbstructLayer::add_sensor_data_handler(std::shared_ptr<SensorDataHandler> sensorDataHandler)
{
    if (sensorDataHandler) {
        m_sensorDataHandlers.push_back(sensorDataHandler);
    } else {
        std::cerr << "Sensor data handler is null" << std::endl;
    }
}

bool BluezAbstructLayer::start_scan()
{
    if (!m_conn) {
        std::cerr << "DBus connection not initialized" << std::endl;
        return false;
    }

    DBusMessage* reply = m_send_dbus_message(m_conn, m_adapter_path, BLUEZ_ADAPTER, "StartDiscovery");
    if (!reply) {
        std::cerr << "Failed to start discovery" << std::endl;
        return false;
    }
    dbus_message_unref(reply);
    return true;
}

bool BluezAbstructLayer::stop_scan()
{
    if (!m_conn) {
        std::cerr << "DBus connection not initialized" << std::endl;
        return false;
    }

    DBusMessage* reply = m_send_dbus_message(m_conn, m_adapter_path, BLUEZ_ADAPTER, "StopDiscovery");
    if (!reply) {
        std::cerr << "Failed to stop discovery" << std::endl;
        return false;
    }
    dbus_message_unref(reply);
    return true;
}

// Create messages in advance
void BluezAbstructLayer::createDbusMessages()
{
    for(auto itr : m_sensorDataHandlers)
    {
        std::string device_path = m_crate_device_path(itr->get_device_mac());
        DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), device_path.c_str(), DBUS_PROPERTIES.c_str(), METHOD_GET_ALL.c_str());
        if (!msg) {
            std::cerr << "Failed to create DBus message\n";
            return;
        }
        const char* interface_name = BLUEZ_DEVICE.c_str();
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface_name, DBUS_TYPE_INVALID);

        std::unique_ptr<DBusMessage, DBusDeleter> uMsg(msg);
        m_dbus_messages.push_back(std::move(uMsg));
    }
}

void BluezAbstructLayer::check_adv_data()
{
    std::vector<uint8_t> byte_data={};

    if (!m_conn) {
        std::cerr << "DBus connection not initialized" << std::endl;
        return;
    }

    auto itr = m_sensorDataHandlers.begin();
    {
        (*itr)->get_device_mac();
        std::string device_path = m_crate_device_path((*itr)->get_device_mac());
        DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), device_path.c_str(), DBUS_PROPERTIES.c_str(), METHOD_GET_ALL.c_str());
        if (!msg) {
            std::cerr << "Failed to create DBus message\n";
            return;
        }

        const char* interface_name = BLUEZ_DEVICE.c_str();
        dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface_name, DBUS_TYPE_INVALID);

        DBusError err;
        dbus_error_init(&err);
        while(true){
            DBusMessage* reply = dbus_connection_send_with_reply_and_block(m_conn, msg, -1, &err);

            if (dbus_error_is_set(&err))
            {
                std::cerr << "DBus error: " << err.message << std::endl;
                dbus_error_free(&err);
                continue;
            }

            if (!reply)
            {
                std::cerr << "Failed to get properties for " << device_path << "\n";
                continue;
            }

            (*itr)->update(reply);
            dbus_message_unref(reply);
            sleep(0.1);

        }dbus_message_unref(msg);
    }

}

std::string BluezAbstructLayer::m_crate_device_path(const std::string device_mac)
{
    std::string device_path = m_adapter_path + "/dev_" + device_mac;
    std::replace(device_path.begin(), device_path.end(), ':', '_');
    return device_path;
}

DBusMessage* BluezAbstructLayer::m_send_dbus_message(DBusConnection* conn, const std::string& path, const std::string& interface, const std::string& method)
{
    DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), path.c_str(), interface.c_str(), method.c_str());
    if (!msg)
    {
        std::cerr << "Failed method call" << std::endl;
        return nullptr;
    }

    DBusError err;
    dbus_error_init(&err);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(conn, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err))
    {
        std::cerr << "DBus error: " << err.message << std::endl;
        dbus_error_free(&err);
        return nullptr;
    }

    return reply;
}

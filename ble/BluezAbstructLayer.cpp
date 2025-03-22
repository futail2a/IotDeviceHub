#include "BluezAbstructLayer.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>

BluezAbstructLayer::BluezAbstructLayer(std::shared_ptr<SensorDataHandler> sensorDataHandler)
{
    m_sensorDataHandler = sensorDataHandler;
    m_conn = nullptr;
    m_adapter_path = BLUEZ_PATH + "/hci0";
    m_device_path = BLUEZ_PATH + "/hci0/dev_" + m_sensorDataHandler->get_device_mac();
    std::replace(m_device_path.begin(), m_device_path.end(), ':', '_');
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
    return true;
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

std::vector<uint8_t> BluezAbstructLayer::get_adv_data()
{
    std::vector<uint8_t> byte_data={};

    if (!m_conn) {
        std::cerr << "DBus connection not initialized" << std::endl;
        return byte_data;
    }

    DBusMessage* msg = dbus_message_new_method_call(BLUEZ_SERVICE.c_str(), m_device_path.c_str(), DBUS_PROPERTIES.c_str(), METHOD_GET_ALL.c_str());
    if (!msg) {
        std::cerr << "Failed to create DBus message\n";
        return byte_data;
    }

    const char* interface_name = BLUEZ_DEVICE.c_str();
    dbus_message_append_args(msg, DBUS_TYPE_STRING, &interface_name, DBUS_TYPE_INVALID);

    DBusError err;
    dbus_error_init(&err);

    DBusMessage* reply = dbus_connection_send_with_reply_and_block(m_conn, msg, -1, &err);
    dbus_message_unref(msg);

    if (dbus_error_is_set(&err))
    {
        std::cerr << "DBus error: " << err.message << std::endl;
        dbus_error_free(&err);
        return byte_data;
    }

    if (!reply)
    {
        std::cerr << "Failed to get properties for " << m_device_path << "\n";
        return byte_data;
    }

    byte_data = m_sensorDataHandler->parse_reply(reply);

    dbus_message_unref(reply);
    return byte_data;
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

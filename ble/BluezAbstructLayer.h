#ifndef BLUEZ_ABSTRUCT_LAYER_H
#define BLUEZ_ABSTRUCT_LAYER_H

#include <string>
#include <vector>
#include <dbus/dbus.h>
#include <unistd.h>
#include <memory>

#include "SensorDataHandler.h"

const std::string BLUEZ_PATH = "/org/bluez";
const std::string BLUEZ_ADAPTER = "org.bluez.Adapter1";
const std::string BLUEZ_DEVICE = "org.bluez.Device1";
const std::string BLUEZ_SERVICE = "org.bluez";
const std::string DBUS_PROPERTIES = "org.freedesktop.DBus.Properties";
const std::string METHOD_GET_ALL = "GetAll";

struct DBusDeleter
{
    void operator()(DBusMessage* msg) const
    {
        if (msg)
        {
            dbus_message_unref(msg);
        }
    }
};

class BluezAbstructLayer
{
public:
    BluezAbstructLayer();
    ~BluezAbstructLayer();

    bool init();
    void add_sensor_data_handler(std::shared_ptr<SensorDataHandler> sensorDataHandler);
    bool start_scan();
    bool stop_scan();
    void check_adv_data();

private:
    std::vector<std::shared_ptr<SensorDataHandler>> m_sensorDataHandlers;
    DBusConnection* m_conn;
    std::string m_adapter_path;
    std::vector<std::unique_ptr<DBusMessage, DBusDeleter>> m_dbus_messages;

    std::string m_crate_device_path(const std::string device_mac);
    DBusMessage* m_send_dbus_message(DBusConnection* conn, const std::string& path, const std::string& interface, const std::string& method);
    std::vector<uint8_t> m_get_service_data(DBusMessageIter* variant_iter);
    std::vector<uint8_t> m_get_variant_byte_array(DBusMessageIter* variant_iter);
    void m_print_byte_array(const std::vector<uint8_t>& data);
    void createDbusMessages();
};

#endif
#ifndef BLUEZ_DBUS_LAYER_H
#define BLUEZ_DBUS_LAYER_H

#include <string>
#include <vector>
#include <dbus/dbus.h>
#include <unistd.h>
#include <memory>

#include "BluetoothAbstructLayer.h"
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

struct DbusMessageContainer
{
    std::unique_ptr<DBusMessage, DBusDeleter> msg;
    std::shared_ptr<SensorDataHandler> handler;
};

class BluezDbusLayer : public BluetoothAbstructLayer
{
public:
    BluezDbusLayer();
    ~BluezDbusLayer();

    bool init() override;
    void add_sensor_data_handler(std::shared_ptr<SensorDataHandler> sensorDataHandler) override;
    bool start_scan() override;
    bool stop_scan() override;
    void check_adv_data() override;

private:
    std::vector<std::shared_ptr<SensorDataHandler>> m_sensorDataHandlers;
    DBusConnection* m_conn;
    std::string m_adapter_path;
    std::vector<DbusMessageContainer> m_dbus_messages;

    std::string m_crate_device_path(const std::string device_mac);
    DBusMessage* m_send_dbus_message(DBusConnection* conn, const std::string& path, const std::string& interface, const std::string& method);
    std::vector<uint8_t> parse_reply(DBusMessage* const reply);
    std::vector<uint8_t> m_get_service_data(DBusMessageIter* const variant_iter);
    std::vector<uint8_t> m_get_variant_byte_array(DBusMessageIter* const variant_iter);

    void m_print_byte_array(const std::vector<uint8_t>& data);
    void createDbusMessages();
};

#endif
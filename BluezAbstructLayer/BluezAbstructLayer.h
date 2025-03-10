#ifndef BLUEZ_ABSTRUCT_LAYER_H
#define BLUEZ_ABSTRUCT_LAYER_H

#include <string>
#include <vector>
#include <dbus/dbus.h>
#include <unistd.h>
#include <memory>

#include "SensorDataParser.h"

const std::string BLUEZ_PATH = "/org/bluez";
const std::string BLUEZ_ADAPTER = "org.bluez.Adapter1";
const std::string BLUEZ_DEVICE = "org.bluez.Device1";
const std::string BLUEZ_SERVICE = "org.bluez";
const std::string DBUS_PROPERTIES = "org.freedesktop.DBus.Properties";
const std::string METHOD_GET_ALL = "GetAll";

class BluezAbstructLayer
{
public:
    BluezAbstructLayer(std::shared_ptr<SensorDataParser> sensorDataParser);
    ~BluezAbstructLayer();

    bool init();
    bool start_scan();
    bool stop_scan();
    std::vector<uint8_t> get_adv_data();

private:
    //TODO: To contain multiple SensorDataParser objects
    std::shared_ptr<SensorDataParser> m_sensorDataParser;
    DBusConnection* m_conn;
    std::string m_adapter_path;
    std::string m_device_path;
    std::string m_destination;

    DBusMessage* m_send_dbus_message(DBusConnection* conn, const std::string& path, const std::string& interface, const std::string& method);

    std::vector<uint8_t> m_get_service_data(DBusMessageIter* variant_iter);
    std::vector<uint8_t> m_get_variant_byte_array(DBusMessageIter* variant_iter);
    void m_print_byte_array(const std::vector<uint8_t>& data);
};

#endif
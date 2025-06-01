#include "BluezAbstructLayer.h"
#include <iostream>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <iomanip>

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
        m_dbus_messages.push_back({std::move(uMsg), itr});
    }
}

void BluezAbstructLayer::check_adv_data()
{
    std::vector<uint8_t> byte_data={};

    if (!m_conn) {
        std::cerr << "DBus connection not initialized" << std::endl;
        return;
    }

    for (auto itr = m_dbus_messages.begin(); itr != m_dbus_messages.end(); ++itr)
    {
        DBusError err;
        dbus_error_init(&err);

        DBusMessage* reply = dbus_connection_send_with_reply_and_block(m_conn, (*itr).msg.get(), -1, &err);
        if (dbus_error_is_set(&err))
        {
            std::cerr << "DBus error: " << err.message << std::endl;
            dbus_error_free(&err);
            break;
        }
        byte_data = parse_reply(reply);
        for(auto handler : m_sensorDataHandlers)
        {
            if(handler->get_device_mac() == (*itr).handler->get_device_mac())
            {
                handler->update(byte_data);
            }
        }
        dbus_message_unref(reply);
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

std::vector<uint8_t> BluezAbstructLayer::parse_reply(DBusMessage* const reply)
{
  std::vector<uint8_t> byte_data;

  DBusMessageIter iter;
  if (!dbus_message_iter_init(reply, &iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
  {
      std::cerr << "Unexpected reply format" << std::endl;
      return byte_data;
  }

  DBusMessageIter sub_iter;
  dbus_message_iter_recurse(&iter, &sub_iter);

  while (dbus_message_iter_get_arg_type(&sub_iter) == DBUS_TYPE_DICT_ENTRY)
  {
      DBusMessageIter entry_iter;
      dbus_message_iter_recurse(&sub_iter, &entry_iter);

      if (dbus_message_iter_get_arg_type(&entry_iter) == DBUS_TYPE_STRING)
      {
          char* key;
          dbus_message_iter_get_basic(&entry_iter, &key);
          dbus_message_iter_next(&entry_iter);

          if (dbus_message_iter_get_arg_type(&entry_iter) == DBUS_TYPE_VARIANT)
          {
              DBusMessageIter variant_iter;
              dbus_message_iter_recurse(&entry_iter, &variant_iter);
              if (std::string(key) == "ServiceData") {
                  DBusMessageIter iter;
                  if (!dbus_message_iter_init(reply, &iter) || dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_ARRAY)
                  {
                      std::cerr << "Unexpected reply format" << std::endl;
                      return byte_data;
                  }
                  DBusMessageIter sub_iter;
                  dbus_message_iter_recurse(&iter, &sub_iter);

                  while (dbus_message_iter_get_arg_type(&sub_iter) == DBUS_TYPE_DICT_ENTRY)
                  {
                      DBusMessageIter entry_iter;
                      dbus_message_iter_recurse(&sub_iter, &entry_iter);

                      if (dbus_message_iter_get_arg_type(&entry_iter) == DBUS_TYPE_STRING)
                      {
                          char* key;
                          dbus_message_iter_get_basic(&entry_iter, &key);
                          dbus_message_iter_next(&entry_iter);

                          if (dbus_message_iter_get_arg_type(&entry_iter) == DBUS_TYPE_VARIANT)
                          {
                              DBusMessageIter variant_iter;
                              dbus_message_iter_recurse(&entry_iter, &variant_iter);
                              if (std::string(key) == "ServiceData") {
                                    byte_data = m_get_service_data(&variant_iter);
                                    //m_print_byte_array(byte_data);
                              }
                          }
                      }
                      dbus_message_iter_next(&sub_iter);
                  }

                  return byte_data;
              }
          }
      }
      dbus_message_iter_next(&sub_iter);
  }
  return byte_data;
}

std::vector<uint8_t> BluezAbstructLayer::m_get_service_data(DBusMessageIter* const variant_iter)
{
  std::vector<uint8_t> byte_data;
  if (dbus_message_iter_get_arg_type(variant_iter) == DBUS_TYPE_ARRAY)
  {
      DBusMessageIter array_iter;
      dbus_message_iter_recurse(variant_iter, &array_iter);

      while (dbus_message_iter_get_arg_type(&array_iter) == DBUS_TYPE_DICT_ENTRY)
      {
          DBusMessageIter dict_entry;
          dbus_message_iter_recurse(&array_iter, &dict_entry);

          if (dbus_message_iter_get_arg_type(&dict_entry) == DBUS_TYPE_STRING)
           {
              char* uuid;
              dbus_message_iter_get_basic(&dict_entry, &uuid);
            //   std::cout << "Service UUID: " << uuid << std::endl;

              dbus_message_iter_next(&dict_entry);

              if (dbus_message_iter_get_arg_type(&dict_entry) == DBUS_TYPE_VARIANT)
              {
                  DBusMessageIter variant_data;
                  dbus_message_iter_recurse(&dict_entry, &variant_data);

                  int var_type = dbus_message_iter_get_arg_type(&variant_data);

                  if (var_type == DBUS_TYPE_ARRAY)
                  {
                        byte_data = m_get_variant_byte_array(&variant_data);
                  }
                  else
                  {
                      std::cout << "ERROR: Expected array of bytes inside variant, but got type "
                                << var_type << std::endl;
                  }
              } else {
                  std::cout << "ERROR: Expected VARIANT after UUID, got type "
                            << dbus_message_iter_get_arg_type(&dict_entry) << std::endl;
              }
          } else {
              std::cout << "ERROR: Expected STRING for UUID key, got type "
                        << dbus_message_iter_get_arg_type(&dict_entry) << std::endl;
          }

          dbus_message_iter_next(&array_iter);
      }
  } else {
      std::cout << "ERROR: Expected ARRAY in ServiceData but got "
                << dbus_message_iter_get_arg_type(variant_iter) << std::endl;
  }

  return byte_data;
}

std::vector<uint8_t> BluezAbstructLayer::m_get_variant_byte_array(DBusMessageIter* const variant_iter)
{
  std::vector<uint8_t> byte_data;

  int arg_type = dbus_message_iter_get_arg_type(variant_iter);

  if (arg_type == DBUS_TYPE_ARRAY) {
      DBusMessageIter array_iter;
      dbus_message_iter_recurse(variant_iter, &array_iter);

      int array_type = dbus_message_iter_get_arg_type(&array_iter);
    //   std::cout << "DEBUG: Inside array, first element type = " << array_type << std::endl;

      if (array_type == DBUS_TYPE_BYTE)
      {
          while (array_type == DBUS_TYPE_BYTE)
          {
              uint8_t byte;
              dbus_message_iter_get_basic(&array_iter, &byte);
              byte_data.push_back(byte);
              dbus_message_iter_next(&array_iter);
              array_type = dbus_message_iter_get_arg_type(&array_iter);
          }
      }
      else
      {
          std::cout << "ERROR: Expected array of bytes, but got type " << array_type << std::endl;
      }
  }
  else
  {
      std::cout << "ERROR: Expected ARRAY but got " << arg_type << std::endl;
  }

  return byte_data;
}


void BluezAbstructLayer::m_print_byte_array(const std::vector<uint8_t>& data) {
    std::cout << "Data: ";
    for (uint8_t byte : data)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    }
    std::cout << std::dec << std::endl;
  }

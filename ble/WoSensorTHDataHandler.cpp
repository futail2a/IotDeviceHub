#include "WoSensorTHDataHandler.h"
#include "MqttTopicList.h"
#include <iostream>
#include <iomanip>

std::vector<uint8_t> WoSensorTHDataHandler::parse_reply(DBusMessage* const reply)
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

std::vector<uint8_t> WoSensorTHDataHandler::m_get_service_data(DBusMessageIter* const variant_iter)
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
              std::cout << "Service UUID: " << uuid << std::endl;

              dbus_message_iter_next(&dict_entry);

              if (dbus_message_iter_get_arg_type(&dict_entry) == DBUS_TYPE_VARIANT)
              {
                  DBusMessageIter variant_data;
                  dbus_message_iter_recurse(&dict_entry, &variant_data);

                  int var_type = dbus_message_iter_get_arg_type(&variant_data);

                  if (var_type == DBUS_TYPE_ARRAY)
                  {
                      byte_data = m_get_variant_byte_array(&variant_data);
                    //   m_print_sensor_data(byte_data);
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

std::vector<uint8_t> WoSensorTHDataHandler::m_get_variant_byte_array(DBusMessageIter* const variant_iter)
{
  std::vector<uint8_t> byte_data;

  int arg_type = dbus_message_iter_get_arg_type(variant_iter);

  if (arg_type == DBUS_TYPE_ARRAY) {
      DBusMessageIter array_iter;
      dbus_message_iter_recurse(variant_iter, &array_iter);

      int array_type = dbus_message_iter_get_arg_type(&array_iter);
      // std::cout << "DEBUG: Inside array, first element type = " << array_type << std::endl;

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

void WoSensorTHDataHandler::m_print_byte_array(const std::vector<uint8_t>& data) {
    std::cout << "Data: ";
    for (uint8_t byte : data)
    {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte << " ";
    }
    std::cout << std::dec << std::endl;
  }

  void WoSensorTHDataHandler::m_print_sensor_data(const std::vector<uint8_t>& data)
  {
  //   m_print_byte_array(data);
    if(data.size() > SERVICEDATA_LEN)
    {
      std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
      return;
    }

    uint8_t temp = data[4]&BIT_0_6_MASK;
    std::cout << "Temperature: " << ((data[4]&BIT_7_MASK) ? "" : "-") << (int)temp << "°C" << std::endl;

    // Notice: API document explains Bit[7] is Templature Scale and Bit[6:0] is Humidity Value
    // But as far as I checked with actual value from device, whole Bit[7:0] expresse humidity value, and Bit[7] is not temperature scale
    // https://github.com/OpenWonderLabs/SwitchBotAPI-BLE/blob/latest/devicetypes/meter.md#broadcast-mode
    uint8_t humid = data[5];
    std::cout << "Humidity: " << (int)humid << "%" << std::endl;
  }

std::vector<MqttMessage> WoSensorTHDataHandler::createPublishMessages(const std::vector<uint8_t>& data)
{
    std::vector<MqttMessage> messages;

    if(data.size() > SERVICEDATA_LEN)
    {
        std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    }
    else
    {
        // TODO: room in topic to be configurable
        std::string temp_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/bed_room" + IOT_TOPIC_SENS_DATA_TEMP;
        uint8_t temp = data[4]&BIT_0_6_MASK;
        std::string signChar = (data[4]&BIT_7_MASK) ? "" : "-";
        std::string temp_str = "Temperature: " + signChar + std::to_string((int)temp) + "°C";
        MqttMessage temp_message{temp_topic_str, temp_str};
        messages.emplace_back(temp_message);

        std::string humid_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/bed_room" + IOT_TOPIC_SENS_DATA_HUMID;
        uint8_t humid = data[5];
        std::string humid_str = "Humidity: " + std::to_string((int)humid) + "%";
        MqttMessage humid_message{humid_topic_str, humid_str};
        messages.emplace_back(humid_message);
    }

    return messages;
}

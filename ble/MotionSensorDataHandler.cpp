#include "MotionSensorDataHandler.h"
#include "MqttTopicList.h"
#include <iostream>
#include <iomanip>

// namespace motion_sensor
// {
    constexpr uint8_t SERVICEDATA_LEN = 6;
    constexpr uint8_t BIT_7_MASK = 0x80;
    constexpr uint8_t BIT_1_0_MASK = 0x03;

void MotionSensorDataHandler::update(DBusMessage* const reply)
{
    if (m_update_cb)
    {
        std::vector<uint8_t> data = parse_reply(reply);
        m_update_cb(data);
    }
}

std::vector<uint8_t> MotionSensorDataHandler::parse_reply(DBusMessage* const reply)
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

std::vector<uint8_t> MotionSensorDataHandler::m_get_service_data(DBusMessageIter* const variant_iter)
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
                      // m_print_sensor_data(byte_data);
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

std::vector<uint8_t> MotionSensorDataHandler::m_get_variant_byte_array(DBusMessageIter* const variant_iter)
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

void MotionSensorDataHandler::m_print_sensor_data(const std::vector<uint8_t>& data)
{
  if(data.size() > SERVICEDATA_LEN)
  {
    std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    return;
  }

  uint8_t pir_time = data[5]&BIT_7_MASK;
  std::cout << "pir_time: " << std::to_string((int)pir_time) << std::endl;

  uint8_t light_intensity = data[5]&BIT_1_0_MASK;
  std::cout << "light_intensity: " << std::to_string((int)light_intensity) << std::endl;
}

std::vector<MqttMessage> MotionSensorDataHandler::createPublishMessages(const std::vector<uint8_t>& data)
{
    std::vector<MqttMessage> messages;

    if(data.size() > SERVICEDATA_LEN)
    {
        std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    }
    else
    {
        // TODO: room in topic to be configurable
        std::string pir_time_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/entrance" + IOT_TOPIC_SENS_DATA_PIR_UTC;
        uint16_t pir_time = (static_cast<uint16_t>(data[3]) << 8) | static_cast<uint16_t>(data[4]);
        std::string pir_time_str = "Since the last trigger PIR time (s): " + std::to_string(pir_time);
        MqttMessage pir_time_message{pir_time_topic_str, pir_time_str};
        messages.emplace_back(pir_time_message);

        // uint8_t pir_time = data[5]&BIT_7_MASK;
        // std::string pir_time_str = "Since the last trigger PIR time (s): " + std::to_string((int)pir_time);
        // MqttMessage pir_time_message{pir_time_topic_str, pir_time_str};
        // std::cout <<pir_time_str << std::endl;
        // messages.emplace_back(pir_time_message);

        std::string light_intensity_topic_str = IOT_TOPIC_SENS_DATA_BASE + "/entrance" + IOT_TOPIC_SENS_DATA_LIGHT_INTENSITY;
        uint8_t light_intensity = data[5]&BIT_1_0_MASK;
        std::string light_intensity_str;
        if(light_intensity==1U)
        {
          light_intensity_str = "Light Intensity: dark";
        }
        else if(light_intensity==2U)
        {
          light_intensity_str = "Light Intensity: bright";
        }
        else
        {
          light_intensity_str = "Light Intensity: unknown (" + std::to_string(light_intensity) + ")";
          std::cerr << "ERROR: Unexpected light intensity value" << std::endl;
        }
        MqttMessage light_intensity_message{light_intensity_topic_str, light_intensity_str};
        messages.emplace_back(light_intensity_message);
    }

    return messages;
}

// }
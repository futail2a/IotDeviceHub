#ifndef SENSOR_DATA_PARSER_H
#define SENSOR_DATA_PARSER_H

#include <vector>

class SensorDataParser
{
public:
 SensorDataParser() = default;
  virtual ~SensorDataParser() = default;
  virtual std::vector<uint8_t> parse_reply(DBusMessage* const reply ) = 0;
  virtual void print_sensor_data(const std::vector<uint8_t>& data) = 0;
  virtual std::string get_device_mac() = 0;
};

#endif
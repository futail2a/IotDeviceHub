#ifndef SENSOR_DATA_PARSER_H
#define SENSOR_DATA_PARSER_H

#include <vector>

class SensorDataParser
{
public:
 SensorDataParser() = default;
  virtual ~SensorDataParser() = default;
  virtual std::vector<uint8_t> parse_reply(DBusMessage* reply ) = 0;
};

#endif
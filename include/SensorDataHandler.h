#ifndef SENSOR_DATA_HANDLER_H
#define SENSOR_DATA_HANDLER_H

#include <vector>

struct MqttMessage
{
    std::string topic;
    std::string message;
};

class SensorDataHandler
{
public:
    SensorDataHandler() = default;
    virtual ~SensorDataHandler() = default;
    virtual std::vector<uint8_t> parse_reply(DBusMessage* const reply ) = 0;
    virtual std::string get_device_mac() = 0;
    virtual std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data) = 0;
};

#endif
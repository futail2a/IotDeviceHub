#ifndef SENSOR_DATA_HANDLER_H
#define SENSOR_DATA_HANDLER_H

#include <vector>
#include <string>
#include <functional>

struct MqttMessage
{
    std::string topic;
    std::string message;
};

using UpdateCb = std::function <void(std::vector<uint8_t>)>;

class SensorDataHandler
{
public:
    SensorDataHandler() = default;
    virtual ~SensorDataHandler() = default;

    virtual void set_update_cb(UpdateCb cb)=0;
    virtual std::vector<uint8_t> parse_reply(DBusMessage* const reply) = 0;
    virtual std::string get_device_mac() = 0;
    virtual std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data) = 0;
    virtual void update(DBusMessage* const reply) = 0;
};

#endif
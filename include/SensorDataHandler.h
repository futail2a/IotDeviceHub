#ifndef SENSOR_DATA_HANDLER_H
#define SENSOR_DATA_HANDLER_H

#include <vector>
#include <string>
#include <functional>
#include <mosquitto.h>

struct MqttMessage
{
    std::string topic;
    std::string message;
    int qos=0;
    bool retain=false;
    mosquitto_property* properties=nullptr;
};

using UpdateCb = std::function <void(std::vector<uint8_t>)>;

class SensorDataHandler
{
public:
    SensorDataHandler() = default;
    virtual ~SensorDataHandler() = default;
    virtual void update(std::vector<uint8_t> &data) = 0;

    virtual void set_update_cb(UpdateCb cb)=0;
    virtual std::string get_device_mac() = 0;
    virtual std::vector<MqttMessage> createPublishMessages(const std::vector<uint8_t>& data) = 0;
};

#endif
#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <MqttTopicList.h>
#include <memory>
#include <mutex>
#include <mosquitto.h>

constexpr uint16_t MQTT_PORT = 1883U;

class MqttManager
{
public:
    MqttManager() = default;
    ~MqttManager() = default;
    bool init();
    void deinit();
    void start();
    void stop();
    bool publishMessage(const std::string topic, const std::string message);

private:
    struct mosquitto* m_mosq;
    std::mutex m_mutex;
};

#endif
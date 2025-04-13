#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <MqttTopicList.h>
#include <memory>
#include <mutex>
#include <mosquitto.h>

constexpr uint16_t MQTT_PORT = 8883U;

class MqttManager
{
public:
    MqttManager() = default;
    ~MqttManager() = default;
    bool init(std::string client_id);
    void deinit();
    void start();
    void stop();
    bool publishMessage(const std::string topic, const std::string message);
    bool subscribe(const std::string topic);

private:
    struct mosquitto* m_mosq;
    std::mutex m_mutex;
};

#endif
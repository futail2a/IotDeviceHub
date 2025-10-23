#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <MqttTopicList.h>
#include <memory>
#include <mutex>
#include <mosquitto.h>
#include "IotEventManager.h"

class MqttManager
{
public:
    MqttManager();
    ~MqttManager() = default;
    bool init(std::string client_id);
    void deinit();
    void start();
    void stop();
    bool publishMessage(const std::string topic, const std::string message, const int qos=0, const bool retain=false, const mosquitto_property *properties=nullptr);
    bool subscribe(const std::string topic, const int qos=0, const int options=0, const mosquitto_property *properties=nullptr);
    void setMediator(std::shared_ptr<IotEventManager> manager) { mMediator = manager; }

    void onMessageReceived(const struct mosquitto_message *msg);

private:
    struct mosquitto* m_mosq;
    std::mutex m_mutex;
    std::shared_ptr<IotEventManager> mMediator;

    std::string mBrokerIpv4="127.0.0.1";
    std::string mCaCertPath="";
    std::string mClientCertPath="";
    std::string mClientKeyPath="";
    std::uint16_t mBrokerPort=1883;
};

#endif
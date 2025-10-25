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
    MqttManager(std::string brokerIpv4="127.0.0.1", std::uint16_t brokerPort=1883U,
                std::string caCertPath="", std::string clientCertPath="", std::string clientKeyPath="")
        : mMosq(nullptr),
          mMutex(),
          mMediator(nullptr),
          mBrokerIpv4(brokerIpv4),
          mBrokerPort(brokerPort),
          mCaCertPath(caCertPath),
          mClientCertPath(clientCertPath),
          mClientKeyPath(clientKeyPath)
    {};

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
    struct mosquitto* mMosq;
    std::mutex mMutex;
    std::shared_ptr<IotEventManager> mMediator;

    std::string mBrokerIpv4;
    std::uint16_t mBrokerPort;
    std::string mCaCertPath;
    std::string mClientCertPath;
    std::string mClientKeyPath;
    std::string mTlsInsecure;
};

#endif
#ifndef I_MQTT_MANAGER_H
#define I_MQTT_MANAGER_H

#include <MqttTopicList.h>
#include <memory>
#include <mutex>
#include <mosquitto.h>

#include "IIotEventManager.h"

class IMqttManager
{
public:
    IMqttManager(std::string brokerIpv4="127.0.0.1", std::uint16_t brokerPort=1883U,
                std::string caCertPath="", std::string clientCertPath="", std::string clientKeyPath="")
        : mMosq(nullptr),
          mMutex(),
          mMediator(nullptr),
          mBrokerIpv4(std::move(brokerIpv4)),
          mBrokerPort(brokerPort),
          mCaCertPath(std::move(caCertPath)),
          mClientCertPath(std::move(clientCertPath)),
          mClientKeyPath(std::move(clientKeyPath))
    {};

    virtual ~IMqttManager() = default;

    virtual bool init(std::string client_id) = 0;
    virtual void deinit() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual bool publishMessage(const std::string topic, const std::string message, const int qos=0, const bool retain=false, const mosquitto_property *properties=nullptr) = 0;
    virtual bool subscribe(const std::string topic, const int qos=0, const int options=0, const mosquitto_property *properties=nullptr) = 0;
    virtual void setMediator(std::shared_ptr<IIotEventManager> manager) = 0;

    virtual void onMessageReceived(const struct mosquitto_message *msg) = 0;

private:
    struct mosquitto* mMosq;
    std::mutex mMutex;
    std::shared_ptr<IIotEventManager> mMediator;

    std::string mBrokerIpv4;
    std::uint16_t mBrokerPort;
    std::string mCaCertPath;
    std::string mClientCertPath;
    std::string mClientKeyPath;
    std::string mTlsInsecure;
};

#endif
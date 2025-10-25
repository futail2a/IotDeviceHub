#include "MqttManager.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "mqtt_protocol.h"
#include "Poco/Util/JSONConfiguration.h"

void on_connect(struct mosquitto *mosq, void *obj, int rc, int flags, const mosquitto_property *props)
{
    if (rc == 0)
    {
        std::cout << "Connected MQTT broker successfully!" << std::endl;
    }
    else
    {
        std::cerr << "Connection failed with code: " << rc << std::endl;
    }
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc, const mosquitto_property *props)
{
    std::cout << "Disconnected with code: " << rc << std::endl;
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg, const mosquitto_property *props)
{
    if(msg == nullptr)
    {
        std::cerr << "Received null message" << std::endl;
        return;
    }

    if(msg->payload == nullptr || msg->payloadlen == 0)
    {
        std::cout << "Received empty message on topic " << msg->topic << std::endl;
    }
    else
    {
        std::cout << "Message received on topic " << msg->topic << ": " << (char *)msg->payload << std::endl;
    }

    auto *manager = static_cast<MqttManager*>(obj);
    if(manager)
    {
        manager->onMessageReceived(msg);
    }

}

void MqttManager::onMessageReceived(const struct mosquitto_message *msg)
{
    if(mMediator)
    {
        std::string eventData="";
        if(msg->payload == nullptr || msg->payloadlen == 0)
        {
            std::cerr << "Received empty message on topic " << msg->topic << std::endl;
        }
        else
        {
            eventData=std::string((char *)msg->payload, msg->payloadlen);
        }
        mMediator->onEvent(msg->topic, eventData);
    }
    else
    {
        std::cerr << "Mediator not set, cannot handle message on topic " << msg->topic << std::endl;
    }
}

bool MqttManager::init(std::string client_id)
{
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if(mosquitto_lib_init()!=MOSQ_ERR_SUCCESS)
        {
            std::cerr << "Failed to initialize mosquitto library" << std::endl;
            return false;
        }
    }

    mMosq = mosquitto_new(client_id.c_str(), true, this);
    if (!mMosq)
    {
        std::cerr << "Failed to create mosquitto instance" << std::endl;
        return false;
    }

    mosquitto_int_option(mMosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);

    mosquitto_connect_v5_callback_set(mMosq, on_connect);
    mosquitto_disconnect_v5_callback_set(mMosq, on_disconnect);
    mosquitto_message_v5_callback_set(mMosq, on_message);

    if(!mCaCertPath.empty())
    {
        const char *cafile = mCaCertPath.c_str();
        const char *capath = nullptr;
        const char *certfile = mClientCertPath.empty() ? nullptr : mClientCertPath.c_str();
        const char *keyfile = mClientKeyPath.empty() ? nullptr : mClientKeyPath.c_str();

        auto ret = mosquitto_tls_set(mMosq, cafile, capath, certfile, keyfile, nullptr);
        if(ret != MOSQ_ERR_SUCCESS)
        {
            std::cerr << "Failed to setup TLS: " << mosquitto_strerror(ret) << " (" << ret << ")" << std::endl;
            mosquitto_destroy(mMosq);
            mMosq = nullptr;
            return false;
        }
    }

    bool tls_insecure = true;
    mosquitto_tls_insecure_set(mMosq, tls_insecure);

    int ret = mosquitto_connect_bind_v5(mMosq, mBrokerIpv4.c_str(), mBrokerPort, 60, nullptr, nullptr);
    if(ret != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to connect to broker: " << mosquitto_strerror(ret) << " (" << ret << ")" << std::endl;
        return false;
    }

    // TODO: Reconnection logic to be implemented

    return true;
}

void MqttManager::deinit()
{
    mosquitto_destroy(mMosq);
    mosquitto_lib_cleanup();
}

void MqttManager::start()
{
    int ret = mosquitto_loop_start(mMosq);
    if (ret != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Error: failed to start mosquitto loop: " << ret << std::endl;
        mosquitto_disconnect_v5(mMosq, MQTT_RC_UNSPECIFIED, nullptr);
        return;
    }
}

void MqttManager::stop()
{
    mosquitto_loop_stop(mMosq, true);
    mosquitto_disconnect_v5(mMosq, MQTT_RC_NORMAL_DISCONNECTION, nullptr);
}

bool MqttManager::publishMessage(const std::string topic, const std::string message, const int qos, const bool retain, const mosquitto_property *properties)
{
    if (!mMosq)
    {
        std::cerr << "Mosquitto instance not initialized" << std::endl;
        return false;
    }

    if (topic.empty())
    {
        std::cerr << "Topic is empty" << std::endl;
        return false;
    }

    if (message.empty())
    {
        std::cerr << "Message is empty" << std::endl;
        return false;
    }

    const char *t = topic.c_str();
    const char *m = message.c_str();
    size_t msize = message.size();
    int messageId = 0;
    auto ret = mosquitto_publish_v5(mMosq, &messageId, t, static_cast<int>(msize), m, qos, retain, properties);

    if (ret != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Error: failed to publish message: " << ret << std::endl;
        mosquitto_disconnect_v5(mMosq, MQTT_RC_UNSPECIFIED, nullptr);
        return false;
    }
    std::cout << "Message published to topic " << topic << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return true;
}

bool MqttManager::subscribe(const std::string topic, const int qos, const int options, const mosquitto_property *properties)
{
    if (!mMosq)
    {
        std::cerr << "Mosquitto instance not initialized" << std::endl;
        return false;
    }

    if (topic.empty())
    {
        std::cerr << "Topic is empty" << std::endl;
        return false;
    }

    int messageId = 0;
    auto res = mosquitto_subscribe_v5(mMosq, &messageId, topic.c_str(), qos, options, properties);
    if(res != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to subscribe to topic: " << topic << std::endl;
        return false;
    }
    std::cout << "Subscribed to topic: " << topic << std::endl;
    return true;
}

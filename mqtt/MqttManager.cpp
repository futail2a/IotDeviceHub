#include "MqttManager.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "mqtt_protocol.h"
#include "Poco/Util/JSONConfiguration.h"

void on_connect(struct mosquitto *mosq, void *obj, int rc, int flags, const mosquitto_property *props) {
    if (rc == 0) {
        std::cout << "Connected MQTT broker successfully!" << std::endl;
    } else {
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

MqttManager::MqttManager()
{
    try
    {
        Poco::Util::JSONConfiguration config = Poco::Util::JSONConfiguration(CONFIG_FILE_PATH);
        mBrokerIpv4 = config.getString("mqtt.brokerIpv4");
        std::cout << "MQTT broker address: " << mBrokerIpv4 << std::endl;
        mBrokerPort = config.getUInt16("mqtt.brokerPort");
        std::cout << "MQTT broker address: " << mBrokerPort << std::endl;

    }
    catch (Poco::Exception& ex)
    {
        std::cerr << "Error: " << ex.displayText() << std::endl;
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
        std::lock_guard<std::mutex> lock(m_mutex);
        if(mosquitto_lib_init()!=MOSQ_ERR_SUCCESS)
        {
            std::cerr << "Failed to initialize mosquitto library" << std::endl;
            return false;
        }
    }

    m_mosq = mosquitto_new(client_id.c_str(), true, this);
    if (!m_mosq)
    {
        std::cerr << "Failed to create mosquitto instance" << std::endl;
        return false;
    }

    mosquitto_int_option(m_mosq, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);

    // mosquitto_log_callback_set(m_mosq, [](struct mosquitto *mosq, void *userdata, int level, const char *str) {
    //     std::cout << "Log: " << str << std::endl;
    // });

    mosquitto_connect_v5_callback_set(m_mosq, on_connect);
    mosquitto_disconnect_v5_callback_set(m_mosq, on_disconnect);
    mosquitto_message_v5_callback_set(m_mosq, on_message);

    // auto ret = mosquitto_tls_set(m_mosq, "/etc/mosquitto/ca_certificates/ca.crt", NULL, "client.crt",  "client.key", NULL);
    auto ret = mosquitto_tls_set(m_mosq, "./cert/indigo/ca.crt", NULL, "./cert/indigo/client.crt",  "./cert/indigo/client.key", NULL);
    if(ret != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to setup TLS " << ret << std::endl;
    }

    mosquitto_tls_insecure_set(m_mosq,true);

    ret = mosquitto_connect_bind_v5(m_mosq, mBrokerIpv4.c_str(), mBrokerPort, 60, nullptr, nullptr);
    if(ret != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to connect to broker: " << ret <<std::endl;
        return false;
    }

    // TODO: Reconnection logic to be implemented

    return true;
}

void MqttManager::deinit()
{
    mosquitto_destroy(m_mosq);
    mosquitto_lib_cleanup();
}

void MqttManager::start()
{
    int ret = mosquitto_loop_start(m_mosq);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: failed to start mosquitto loop: " << ret << std::endl;
        mosquitto_disconnect_v5(m_mosq, MQTT_RC_UNSPECIFIED, nullptr);
        return;
    }
}

void MqttManager::stop()
{
    mosquitto_loop_stop(m_mosq, true);
    mosquitto_disconnect_v5(m_mosq, MQTT_RC_NORMAL_DISCONNECTION, nullptr);
}

bool MqttManager::publishMessage(const std::string topic, const std::string message, const int qos, const bool retain, const mosquitto_property *properties)
{
    if (!m_mosq) {
        std::cerr << "Mosquitto instance not initialized" << std::endl;
        return false;
    }

    if (topic.empty()) {
        std::cerr << "Topic is empty" << std::endl;
        return false;
    }

    if (message.empty()) {
        std::cerr << "Message is empty" << std::endl;
        return false;
    }

    const char *t = topic.c_str();
    const char *m = message.c_str();
    size_t msize = message.size();
    int messageId = 0;
    auto ret = mosquitto_publish_v5(m_mosq, &messageId, t, static_cast<int>(msize), m, qos, retain, properties);

    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: failed to publish message: " << ret << std::endl;
        mosquitto_disconnect_v5(m_mosq, MQTT_RC_UNSPECIFIED, nullptr);
        return false;
    }
    std::cout << "Message published to topic " << topic << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return true;
}

bool MqttManager::subscribe(const std::string topic, const int qos, const int options, const mosquitto_property *properties)
{
    if (!m_mosq) {
        std::cerr << "Mosquitto instance not initialized" << std::endl;
        return false;
    }

    if (topic.empty()) {
        std::cerr << "Topic is empty" << std::endl;
        return false;
    }

    int messageId = 0;
    auto res = mosquitto_subscribe_v5(m_mosq, &messageId, topic.c_str(), qos, options, properties);
    if(res != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to subscribe to topic: " << topic << std::endl;
        return false;
    }
    std::cout << "Subscribed to topic: " << topic << std::endl;
    return true;
}

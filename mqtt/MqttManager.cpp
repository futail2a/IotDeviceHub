#include "MqttManager.h"

#include <iostream>
#include <thread>
#include <chrono>

void on_connect(struct mosquitto *mosq, void *obj, int rc) {
    if (rc == 0) {
        std::cout << "Connected successfully!" << std::endl;
    } else {
        std::cerr << "Connection failed with code: " << rc << std::endl;
    }
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc) {
    std::cout << "Disconnected with code: " << rc << std::endl;
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    std::cout << "Message received on topic " << msg->topic << ": " << (char *)msg->payload << std::endl;
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

    m_mosq = mosquitto_new(client_id.c_str(), true, NULL);
    if (!m_mosq)
    {
        std::cerr << "Failed to create mosquitto instance" << std::endl;
        return false;
    }

    mosquitto_log_callback_set(m_mosq, [](struct mosquitto *mosq, void *userdata, int level, const char *str) {
        std::cout << "Log: " << str << std::endl;
    });
    mosquitto_connect_callback_set(m_mosq, on_connect);
    mosquitto_disconnect_callback_set(m_mosq, on_disconnect);
    mosquitto_message_callback_set(m_mosq, on_message);

    auto ret = mosquitto_tls_set(m_mosq, "/etc/mosquitto/ca_certificates/ca.crt", NULL, "client.crt",  "client.key", NULL);
    if(ret != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to setup TLS " << ret << std::endl;
    }

    mosquitto_tls_insecure_set(m_mosq,true);

    ret = mosquitto_connect(m_mosq, "127.0.0.1", MQTT_PORT, 60);
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
        mosquitto_disconnect(m_mosq);
        return;
    }
}

void MqttManager::stop()
{
    mosquitto_loop_stop(m_mosq, true);
    mosquitto_disconnect(m_mosq);
}

bool MqttManager::publishMessage(const std::string topic, const std::string message)
{
    const char *t = topic.c_str();
    const char *m = message.c_str();
    size_t msize = message.size();
    int messageId = 0;
    auto ret = mosquitto_publish(m_mosq, &messageId, t, static_cast<int>(msize), m, 0, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        std::cerr << "Error: failed to publish message: " << ret << std::endl;
        mosquitto_disconnect(m_mosq);
        return false;
    }
    std::cout << "Message published to topic " << topic << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return true;
}

bool MqttManager::subscribe(const std::string topic)
{
    auto res = mosquitto_subscribe(m_mosq, nullptr, topic.c_str(), 0);
    if(res != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to subscribe to topic: " << topic << std::endl;
        return false;
    }
    std::cout << "Subscribed to topic: " << topic << std::endl;
    return true;
}

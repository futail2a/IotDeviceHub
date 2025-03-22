#include "MqttManager.h"

#include <iostream>
#include <thread>
#include <chrono>

bool MqttManager::init()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(mosquitto_lib_init()!=MOSQ_ERR_SUCCESS)
        {
            std::cerr << "Failed to initialize mosquitto library" << std::endl;
            return false;
        }
    }

    m_mosq = mosquitto_new("iot_device_hub", true, nullptr);
    if (!m_mosq)
    {
        std::cerr << "Failed to create mosquitto instance" << std::endl;
        return false;
    }

    if (mosquitto_connect(m_mosq, "localhost", MQTT_PORT, 60) != MOSQ_ERR_SUCCESS)
    {
        std::cerr << "Failed to connect to broker" << std::endl;
        return false;
    }

    // TODO: Reconnection logic to be implemented

    return true;
}

void MqttManager::start()
{
    mosquitto_loop_start(m_mosq);
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
        std::cerr << "Error: failed to publish message" << std::endl;
        mosquitto_disconnect(m_mosq);
        return false;
    }
    std::cout << "Message published to topic " << topic << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return true;
}

MqttManager::~MqttManager()
{
    mosquitto_destroy(m_mosq);
    mosquitto_lib_cleanup();
}

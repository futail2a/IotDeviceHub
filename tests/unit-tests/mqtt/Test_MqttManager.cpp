#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "MqttManager.h"

const std::string DEFAULT_MAC = "00:00:00:00:00:00";

TEST(Test_MqttManager, init)
{
    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
    EXPECT_TRUE(true);
}

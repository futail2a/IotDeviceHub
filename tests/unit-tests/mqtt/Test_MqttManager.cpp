#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "MqttManager.h"
#include "IotEventManagerMock.h"
#include "MosquittoMock.h"

const std::string DEFAULT_MAC = "00:00:00:00:00:00";

TEST(Test_MqttManager, init)
{
    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
    EXPECT_TRUE(true);//todo
}

// TEST(Test_MqttManager, onMessageReceived)
// {
//     MqttManager mqttManager;
//     mqttManager.init("iot_device_hub");
//     auto eventManagerMock = std::make_shared<IotEventManager>();
//     mqttManager.setMediator(eventManagerMock);

//     EXPECT_CALL(*eventManagerMock, onEvent(testing::_, testing::_)).Times(1);

//     testing::internal::CaptureStdout();
//     mqttManager.onMessageReceived(nullptr);
//     std::string output = testing::internal::GetCapturedStdout();
//     EXPECT_STREQ("Received empty message on topic \n", output.c_str());
// }

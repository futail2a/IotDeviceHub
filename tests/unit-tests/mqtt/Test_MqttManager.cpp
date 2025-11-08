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
using ::testing::_;

TEST(Test_MqttManager, init)
{
    MosquittoMock mosqMock;
    EXPECT_CALL(mosqMock, mosquitto_lib_init())
        .WillOnce(testing::Return(MOSQ_ERR_SUCCESS));

    EXPECT_CALL(mosqMock, mosquitto_new(_, _, _))
        .WillOnce(testing::Return(reinterpret_cast<struct mosquitto*>(0x1)));

    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
}

// TEST(Test_MqttManager, onMessageReceived)
// {
//     MqttManager mqttManager;
//     mqttManager.init("iot_device_hub");
//     std::shared_ptr<IotEventManager> eventManagerMock = std::make_shared<IotEventManager>();
//     mqttManager.setMediator(eventManagerMock);

//     EXPECT_CALL(*eventManagerMock, onEvent(testing::_, testing::_)).Times(1);

//     testing::internal::CaptureStdout();
//     mqttManager.onMessageReceived(nullptr);
//     std::string output = testing::internal::GetCapturedStdout();
//     EXPECT_STREQ("Received empty message on topic \n", output.c_str());
// }

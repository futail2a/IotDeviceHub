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

TEST(Test_MqttManager, on_connection_ok)
{
    MosquittoMock mosqMock;
    EXPECT_CALL(mosqMock, mosquitto_lib_init())
        .WillOnce(testing::Return(MOSQ_ERR_SUCCESS));

    EXPECT_CALL(mosqMock, mosquitto_new(_, _, _))
        .WillOnce(testing::Return(reinterpret_cast<struct mosquitto*>(0x1)));

    void (*saved_callback)(struct mosquitto *, void *, int , int , const mosquitto_property *) = nullptr;

    EXPECT_CALL(mosqMock, mosquitto_connect_v5_callback_set(_, _))
        .WillOnce(testing::DoAll(
            testing::SaveArg<1>(&saved_callback),
            testing::Return()
        ));

    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
    EXPECT_NE(saved_callback, nullptr);
    
    testing::internal::CaptureStdout();
    saved_callback(nullptr, nullptr, 0, 0, nullptr);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Connected MQTT broker successfully!\n", output.c_str());
}

TEST(Test_MqttManager, on_connection_failed)
{
    MosquittoMock mosqMock;
    EXPECT_CALL(mosqMock, mosquitto_lib_init())
        .WillOnce(testing::Return(MOSQ_ERR_SUCCESS));

    EXPECT_CALL(mosqMock, mosquitto_new(_, _, _))
        .WillOnce(testing::Return(reinterpret_cast<struct mosquitto*>(0x1)));

    void (*saved_callback)(struct mosquitto *, void *, int , int , const mosquitto_property *) = nullptr;

    EXPECT_CALL(mosqMock, mosquitto_connect_v5_callback_set(_, _))
        .WillOnce(testing::DoAll(
            testing::SaveArg<1>(&saved_callback),
            testing::Return()
        ));

    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
    EXPECT_NE(saved_callback, nullptr);

    testing::internal::CaptureStdout();
    saved_callback(nullptr, nullptr, 1, 0, nullptr);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Connection failed with code: 1\n", output.c_str());
}

TEST(Test_MqttManager, on_disconnect)
{
    MosquittoMock mosqMock;
    EXPECT_CALL(mosqMock, mosquitto_lib_init())
        .WillOnce(testing::Return(MOSQ_ERR_SUCCESS));

    EXPECT_CALL(mosqMock, mosquitto_new(_, _, _))
        .WillOnce(testing::Return(reinterpret_cast<struct mosquitto*>(0x1)));

    void (*saved_callback)(struct mosquitto *, void * , int , const mosquitto_property *) = nullptr;

    EXPECT_CALL(mosqMock, mosquitto_disconnect_v5_callback_set(_, _))
        .WillOnce(testing::DoAll(
            testing::SaveArg<1>(&saved_callback),
            testing::Return()
        ));

    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
    EXPECT_NE(saved_callback, nullptr);

    testing::internal::CaptureStdout();
    saved_callback(nullptr, nullptr, 0, nullptr);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Disconnected with code: 0\n", output.c_str());
}

TEST(Test_MqttManager, mosquitto_lib_init_fail)
{
    MosquittoMock mosqMock;
    EXPECT_CALL(mosqMock, mosquitto_lib_init())
        .WillOnce(testing::Return(MOSQ_ERR_INVAL));

    EXPECT_CALL(mosqMock, mosquitto_new(_, _, _)).Times(0);

    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
}

TEST(Test_MqttManager, mosquitto_new_fail)
{
    MosquittoMock mosqMock;
    EXPECT_CALL(mosqMock, mosquitto_lib_init())
        .WillOnce(testing::Return(MOSQ_ERR_SUCCESS));

    EXPECT_CALL(mosqMock, mosquitto_new(_, _, _))
        .WillOnce(testing::Return(nullptr));

    EXPECT_CALL(mosqMock, mosquitto_int_option(_, _, _)).Times(0);

    MqttManager mqttManager;
    mqttManager.init("iot_device_hub");
}

TEST(Test_MqttManager, onMessageReceived_with_payload)
{
    MqttManager mqttManager;
    auto eventManagerMock = std::make_shared<IotEventManagerMock>();
    mqttManager.setMediator(eventManagerMock);
    EXPECT_CALL(*eventManagerMock, onEvent(testing::_, testing::_)).Times(1);

    testing::internal::CaptureStdout();
    mosquitto_message msg;
    msg.topic = const_cast<char*>("test/topic");
    const char* payload = "test!";
    msg.payload = const_cast<char*>(payload);
    msg.payloadlen = static_cast<int>(strlen(payload));
    mqttManager.onMessageReceived(&msg);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Received message with payload on topic test/topic\n", output.c_str());
}

TEST(Test_MqttManager, onMessageReceived_without_payload)
{
    MqttManager mqttManager;
    auto eventManagerMock = std::make_shared<IotEventManagerMock>();
    mqttManager.setMediator(eventManagerMock);
    EXPECT_CALL(*eventManagerMock, onEvent(testing::_, testing::_)).Times(1);

    testing::internal::CaptureStdout();
    mosquitto_message msg;
    msg.topic = const_cast<char*>("topic/empty");
    msg.payload = nullptr;
    msg.payloadlen = 0U;
    mqttManager.onMessageReceived(&msg);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Received empty message on topic topic/empty\n", output.c_str());
}

TEST(Test_MqttManager, onMessageReceived_without_mediator)
{
    MqttManager mqttManager;

    auto eventManagerMock = std::make_shared<IotEventManagerMock>();
    EXPECT_CALL(*eventManagerMock, onEvent(testing::_, testing::_)).Times(0);

    testing::internal::CaptureStdout();
    mosquitto_message msg;
    msg.topic = const_cast<char*>("test/topic");
    const char* payload = "test!";
    msg.payload = const_cast<char*>(payload);
    msg.payloadlen = static_cast<int>(strlen(payload));
    mqttManager.onMessageReceived(&msg);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Mediator not set, cannot handle message on topic test/topic\n", output.c_str());
}

TEST(Test_MqttManager, onMessageReceived_without_topic)
{
    MqttManager mqttManager;

    auto eventManagerMock = std::make_shared<IotEventManagerMock>();
    mqttManager.setMediator(eventManagerMock);
    EXPECT_CALL(*eventManagerMock, onEvent(testing::_, testing::_)).Times(0);

    testing::internal::CaptureStdout();
    mosquitto_message msg;
    msg.topic = nullptr;
    const char* payload = "test!";
    msg.payload = const_cast<char*>(payload);
    msg.payloadlen = static_cast<int>(strlen(payload));
    mqttManager.onMessageReceived(&msg);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Received message with null topic\n", output.c_str());
}
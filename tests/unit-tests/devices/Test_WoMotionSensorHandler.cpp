#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "WoMotionSensorHandler.h"

const std::string DEFAULT_MAC = "00:00:00:00:00:00";

TEST(Test_WoMotionSensorHandler, getMacAddr)
{
    WoMotionSensorHandler handler(DEFAULT_MAC);
    EXPECT_STREQ(handler.getMacAddr().c_str(), DEFAULT_MAC.c_str());
}

TEST(Test_WoMotionSensorHandler, onConnected)
{
    WoMotionSensorHandler handler(DEFAULT_MAC);

    testing::internal::CaptureStdout();
    handler.onConnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Motion Sensor is connected\n", output.c_str());
}

TEST(Test_WoMotionSensorHandler, onDiscconnected)
{
    WoMotionSensorHandler handler(DEFAULT_MAC);

    testing::internal::CaptureStdout();
    handler.onDisconnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Motion Sensor is disconnected\n", output.c_str());
}

TEST(Test_WoMotionSensorHandler, set_and_get_State)
{
    WoMotionSensorHandler handler(DEFAULT_MAC);

    handler.setState(BleDeviceState::CONNECTED);
    EXPECT_EQ(handler.getState(), BleDeviceState::CONNECTED);

    handler.setState(BleDeviceState::DISCONNECTED);
    EXPECT_EQ(handler.getState(), BleDeviceState::DISCONNECTED);
}

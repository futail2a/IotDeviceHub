#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "WoHandHandler.h"

const std::string DEFAULT_MAC = "00:00:00:00:00:00";

TEST(Test_WoHandHandler, getMacAddr)
{
    WoHandHandler handler(DEFAULT_MAC);
    EXPECT_STREQ(handler.getMacAddr().c_str(), DEFAULT_MAC.c_str());
}

TEST(Test_WoHandHandler, onConnected)
{
    WoHandHandler handler(DEFAULT_MAC);

    testing::internal::CaptureStdout();
    handler.onConnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Bot is connected\n", output.c_str());
}

TEST(Test_WoHandHandler, onDiscconnected)
{
    WoHandHandler handler(DEFAULT_MAC);

    testing::internal::CaptureStdout();
    handler.onDisconnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Bot is disconnected\n", output.c_str());
}

TEST(Test_WoHandHandler, getExecActionCommand)
{
    WoHandHandler handler(DEFAULT_MAC);
    BleCommand command = handler.getExecActionCommand();

    EXPECT_STREQ(command.macAddr.c_str(), DEFAULT_MAC.c_str());
    EXPECT_STREQ(command.charPath.c_str(), "/service0011/char0015");
    EXPECT_STREQ(command.method.c_str(), "WriteValue");
    std::vector<uint8_t> expectedData = {0x57, 0x01, 0x00};
    EXPECT_EQ(command.data, expectedData);
    EXPECT_EQ(command.options["type"],"command");
}


TEST(Test_WoHandHandler, set_and_get_State)
{
    WoHandHandler handler(DEFAULT_MAC);

    handler.setState(BleDeviceState::CONNECTED);
    EXPECT_EQ(handler.getState(), BleDeviceState::CONNECTED);

    handler.setState(BleDeviceState::DISCONNECTED);
    EXPECT_EQ(handler.getState(), BleDeviceState::DISCONNECTED);
}

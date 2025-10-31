#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "WoBulbHandler.h"

const std::string DEFAULT_MAC = "00:00:00:00:00:00";

TEST(Test_WoBulbHandler, getMacAddr)
{
    WoBulbHandler handler(DEFAULT_MAC);
    EXPECT_STREQ(handler.getMacAddr().c_str(), DEFAULT_MAC.c_str());
}

TEST(Test_WoBulbHandler, onConnected)
{
    WoBulbHandler handler(DEFAULT_MAC);

    testing::internal::CaptureStdout();
    handler.onConnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Color Bulb is connected\n", output.c_str());
}

TEST(Test_WoBulbHandler, onDiscconnected)
{
    WoBulbHandler handler(DEFAULT_MAC);

    testing::internal::CaptureStdout();
    handler.onDisconnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Color Bulb is disconnected\n", output.c_str());
}

TEST(Test_WoBulbHandler, getTurnOnCommand)
{
    WoBulbHandler handler(DEFAULT_MAC);
    BleCommand command = handler.getTurnOnCommand();

    EXPECT_STREQ(command.macAddr.c_str(), DEFAULT_MAC.c_str());
    EXPECT_STREQ(command.charPath.c_str(), "/service0028/char002c");
    EXPECT_STREQ(command.method.c_str(), "WriteValue");
    std::vector<uint8_t> expectedData = {0x57, 0x0F, 0x47, 0x01, 0x01};
    EXPECT_EQ(command.data, expectedData);
    EXPECT_EQ(command.options["type"],"command");
}

TEST(Test_WoBulbHandler, getTurnOffCommand)
{
    WoBulbHandler handler(DEFAULT_MAC);
    BleCommand command = handler.getTurnOffCommand();

    EXPECT_STREQ(command.macAddr.c_str(), DEFAULT_MAC.c_str());
    EXPECT_STREQ(command.charPath.c_str(), "/service0028/char002c");
    EXPECT_STREQ(command.method.c_str(), "WriteValue");
    std::vector<uint8_t> expectedData = {0x57, 0x0F, 0x47, 0x01, 0x02};
    EXPECT_EQ(command.data, expectedData);
    EXPECT_EQ(command.options["type"],"command");
}

TEST(Test_WoBulbHandler, set_and_get_State)
{
    WoBulbHandler handler(DEFAULT_MAC);

    handler.setState(BleDeviceState::CONNECTED);
    EXPECT_EQ(handler.getState(), BleDeviceState::CONNECTED);

    handler.setState(BleDeviceState::DISCONNECTED);
    EXPECT_EQ(handler.getState(), BleDeviceState::DISCONNECTED);
}

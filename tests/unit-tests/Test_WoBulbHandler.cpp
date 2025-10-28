#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>

#include "WoBulbHandler.h"

TEST(Test_WoBulbHandler, onConnected) {
    WoBulbHandler handler("00:00:00:00:00:00");

    testing::internal::CaptureStdout();
    handler.onConnected();
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_STREQ("Color Bulb is connected\n", output.c_str());
}
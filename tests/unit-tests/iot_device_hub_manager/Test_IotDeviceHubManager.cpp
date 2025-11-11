#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <map>
#include <string>
#include <vector>
#include <memory>
#include <thread>

#include "IotDeviceHubManager.h"
#include "IotEventManagerMock.h"

TEST(Test_IotDeviceHubManager, getMacAddr)
{
    IotDeviceHubManager manager;
    EXPECT_TRUE(manager.init());
}

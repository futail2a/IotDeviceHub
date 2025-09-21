#include "BleAbstructLayer.h"
#include "WoMotionSensorHandler.h"
#include "WoBulbHandler.h"
#include "WoHandHandler.h"
#include "MqttManager.h"
#include "IotEventManager.h"

#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>
#include <atomic>

#include "Poco/Timer.h"

const long DEFAULT_LIGHT_INTERVAL = 10000; // 10 seconds

class IotDeviceHubManager
{
public:
    IotDeviceHubManager();
    ~IotDeviceHubManager(){};

    bool init();
    void run();
    void stop();
    void terminate();
    void notify(const std::vector<uint8_t>& data);

private:
    std::unique_ptr<BleAbstructLayer> mBle;
    std::unique_ptr<MqttManager> mMqtt;
    std::shared_ptr<IotEventManager> mEventManager;

    // std::shared_ptr<SensorDataHandler> m_th_sensor_data_handler;
    std::shared_ptr<WoMotionSensorHandler> mMotionSensorDevice;
    std::shared_ptr<WoBulbHandler> mBulbDevice;
    std::shared_ptr<WoHandHandler> mBotDevice;
    std::atomic<bool> isRunning{true};

    void onMotionUpdate(std::vector<uint8_t> data);

    std::unique_ptr<Poco::Timer> mLightTimer;
    void onLightTimeout(Poco::Timer& timer);
};
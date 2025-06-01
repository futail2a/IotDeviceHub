#include "BluetoothAbstructLayer.h"
#include "WoSensorTHDataHandler.h"
#include "MotionSensorDataHandler.h"
#include "SensorDataHandler.h"
#include "MqttManager.h"
#include <iostream>
#include <iomanip>
#include <memory>
#include <vector>

class IotDeviceHubManager
{
public:
    IotDeviceHubManager();
    ~IotDeviceHubManager(){};

    bool init();
    void run();
    void stop();
    void notify(const std::vector<uint8_t>& data);

private:
    std::shared_ptr<SensorDataHandler> m_th_sensor_data_handler;
    std::shared_ptr<SensorDataHandler> m_motion_sensor_data_handler;
    std::unique_ptr<BluetoothAbstructLayer> m_bluetooth;
    std::unique_ptr<MqttManager> m_mqtt;

    void on_th_update(std::vector<uint8_t> data);
    void on_motion_update(std::vector<uint8_t> data);
};
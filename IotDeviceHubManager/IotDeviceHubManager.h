#include "BluezAbstructLayer.h"
#include "WoSensorTHDataHandler.h"
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
    std::unique_ptr<BluezAbstructLayer> m_bluez;
    std::unique_ptr<MqttManager> m_mqtt;

    void on_th_update(std::vector<uint8_t> data);
};
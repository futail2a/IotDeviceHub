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
    ~IotDeviceHubManager();

    bool init();
    void run();
    void notify(const std::vector<uint8_t>& data);

private:
    std::shared_ptr<SensorDataHandler> m_sensorDataHandler;
    std::unique_ptr<BluezAbstructLayer> m_bluez;
    std::unique_ptr<MqttManager> m_mqtt;

};
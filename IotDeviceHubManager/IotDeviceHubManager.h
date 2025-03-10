#include "BluezAbstructLayer.h"
#include <iostream>
#include "SwtichBotApiDataParser.h"
#include "SensorObserver.h"
#include <iomanip>
#include <memory>
#include <vector>

#include "SwtichBotApiDataParser.h"
#include "SwitchBotObserver.h"
class IotDeviceHubManager
{
public:
    IotDeviceHubManager()
    {
        //TODO: Create factory method for IotDeviceHubManager to specify IoT device
        m_bluez =std::make_unique<BluezAbstructLayer>(std::make_shared<WoSensorTHDataParser>());
        std::shared_ptr<SensorObserver> observer = std::make_shared<WoSensorTHObserver>();
        addObserver(observer);
    };
    ~IotDeviceHubManager(){};

    void run();

    void addObserver(const std::shared_ptr<SensorObserver> observer);
    void removeObserver(const std::shared_ptr<SensorObserver> observer);
    void notify(const std::vector<uint8_t>& data);

private:
    std::unique_ptr<BluezAbstructLayer> m_bluez;
    std::vector<std::shared_ptr<SensorObserver>> observers;

};
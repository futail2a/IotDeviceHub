#include "SwitchBotObserver.h"

#include <vector>
#include <memory>
#include <iostream>
#include <iomanip>

void WoSensorTHObserver::update(const std::vector<uint8_t>& data)
{
    std::cout << "WoSensorTHObserver::update" << std::endl;
}

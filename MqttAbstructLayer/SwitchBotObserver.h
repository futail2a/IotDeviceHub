#ifndef SWICTH_BOT_OBSERVER_H
#define SWICTH_BOT_OBSERVER_H

#include <vector>
#include <memory>
#include <iostream>

#include "SensorObserver.h"

class WoSensorTHObserver : public SensorObserver
{
    public:
    void update(const std::vector<uint8_t>& data) override;
};

#endif
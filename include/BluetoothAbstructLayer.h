#ifndef BLUETOOTH_ABSTRUCT_LAYER_H
#define BLUETOOTH_ABSTRUCT_LAYER_H

#include <vector>
#include <memory>
#include <iostream>

#include "SensorDataHandler.h"

class BluetoothAbstructLayer
{
public:
    virtual ~BluetoothAbstructLayer() = default;

    virtual bool init() = 0;
    virtual bool start_scan() = 0;
    virtual bool stop_scan() = 0;
    virtual void check_adv_data() = 0;
    virtual void add_sensor_data_handler(std::shared_ptr<SensorDataHandler> sensorDataHandler)  = 0;
};

#endif
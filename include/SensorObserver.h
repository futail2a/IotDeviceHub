#ifndef SENSOROBSERVER_H
#define SENSOROBSERVER_H

#include <vector>
#include <memory>
#include <iostream>

class SensorObserver
{
public:
    virtual void update(const std::vector<uint8_t> &data) = 0;
    virtual ~SensorObserver() = default;
};

#endif
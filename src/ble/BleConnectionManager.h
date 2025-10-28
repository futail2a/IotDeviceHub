#ifndef BLE_CONNECTION_MANAGER_H
#define BLE_CONNECTION_MANAGER_H

#include "BleDeviceHandler.h"

class BleConnectionManager
{
public:
    BleConnectionManager() = default;
    virtual ~BleConnectionManager() = default;

    virtual bool init() = 0;
    virtual void terminate() = 0;
    virtual void connect() = 0;
    virtual void setDevice(std::shared_ptr<BleDeviceHandler> device) = 0;
    virtual void sendCommand(const BleCommand& command) = 0;
};

#endif
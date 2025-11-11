#ifndef I_BLE_MANAGER_H
#define I_BLE_MANAGER_H

#include "BleDeviceHandler.h"

class IBleManager
{
public:
    IBleManager() = default;
    virtual ~IBleManager() = default;

    virtual bool init() = 0;
    virtual void terminate() = 0;
    virtual bool startScan() = 0;
    virtual bool stopScan() = 0;
    virtual void setDevice(std::shared_ptr<BleDeviceHandler> device) = 0;
    virtual void connectDevices() = 0;
    virtual void sendCommand(const BleCommand& command) = 0;
};

#endif
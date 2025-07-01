#ifndef BLE_SCAN_MANAGER_H
#define BLE_SCAN_MANAGER_H

#include "BleDeviceHandler.h"

//TODO change name to BlePacketSniffer
class BleScanManager
{
public:
    BleScanManager() = default;
    virtual ~BleScanManager() = default;

    virtual bool init() = 0;
    virtual void terminate() = 0;
    virtual bool startScan() = 0;
    virtual bool stopScan() = 0;
    virtual void setDevice(std::shared_ptr<BleDeviceHandler> deviceHandler) = 0;
};

#endif
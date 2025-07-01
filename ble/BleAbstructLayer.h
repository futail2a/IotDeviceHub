#ifndef BLE_ABSTRUCT_LAYER_H
#define BLE_ABSTRUCT_LAYER_H

#include <vector>
#include <memory>
#include <iostream>

#include "BleDeviceHandler.h"
#include "BleScanManager.h"
#include "BleConnectionManager.h"

class BleAbstructLayer
{
public:
    BleAbstructLayer() = default;
    ~BleAbstructLayer() = default;

    bool init(std::unique_ptr<BleScanManager> scanMgr, std::unique_ptr<BleConnectionManager> connectionMgr);
    void terminate();
    bool start();
    bool stop();
    void connectDevices();
    void sendBleCommand(const BleCommand& command);
    void registerScannedDevice(std::shared_ptr<BleDeviceHandler> device);
    void registerConnectDevice(std::shared_ptr<BleDeviceHandler> device);

private:
    std::unique_ptr<BleScanManager> mScanManager;
    std::unique_ptr<BleConnectionManager> mConnectionManager;
};

#endif
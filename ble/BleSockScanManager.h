#ifndef BLE_SOCK_SCAN_MANAGER_H
#define BLE_SOCK_SCAN_MANAGER_H

#include "BleScanManager.h"
#include "BleDeviceHandler.h"

#include <vector>
#include <memory>
#include <atomic>
#include <thread>

constexpr uint8_t MAC_LENGTH = 6;
constexpr uint8_t SERVICE_DATA_TYPE = 0x16;
constexpr uint8_t SERVICE_DATA_INDEX = 4;
constexpr long SOCK_TIMEOUT_USEC = 100000; // 100 ms

class BleSockScanManager : public BleScanManager
{
public:
    BleSockScanManager() = default;
    ~BleSockScanManager();

    bool init() override;
    void terminate() override;
    bool startScan() override;
    bool stopScan() override;
    void setDevice(std::shared_ptr<BleDeviceHandler> deviceHandler) override;

    void scanning();

private:
    std::vector<std::pair<std::shared_ptr<BleDeviceHandler>,std::string>> mBleDeviceHandlerMacPairs; // pari of handler and converted mac address
    int mSock=-1;
    const uint8_t type = 0x01; // LE scan
    const uint16_t interval = 0x0010; // 10 ms
    const uint16_t window = 0x0010; // 10 ms
    const uint8_t own_type = 0x00; // Public address
    const uint8_t filter = 0x00; // No filter
    const int to = 1000; // Timeout in milliseconds

    std::atomic<bool> isScanning{false};
    std::thread mScanningThread;

    std::string toLowerMac(const uint8_t* addr);

};

#endif
#include "BleAbstructLayer.h"

bool BleAbstructLayer::init(std::unique_ptr<BleScanManager> scanMgr, std::unique_ptr<BleConnectionManager> connectionMgr)
{
    if (!scanMgr)
    {
        std::cerr << "Scan manager is null" << std::endl;
        return false;
    }
    mScanManager = std::move(scanMgr);

    if (!connectionMgr)
    {
        std::cerr << "Connection manager is null" << std::endl;
        return false;
    }
    mConnectionManager = std::move(connectionMgr);

    return mScanManager->init() && mConnectionManager->init();
}

void BleAbstructLayer::terminate()
{
    if (mScanManager)
    {
        mScanManager->terminate();
    }
    if (mConnectionManager)
    {
        mConnectionManager->terminate();
    }
}

bool BleAbstructLayer::start()
{
    if (!mScanManager)
    {
        std::cerr << "Scan manager is not initialized" << std::endl;
        return false;
    }
    return mScanManager->startScan();
}

bool BleAbstructLayer::stop()
{
    if (!mScanManager)
    {
        std::cerr << "Scan manager is not initialized" << std::endl;
        return false;
    }
    return mScanManager->stopScan();
}

void BleAbstructLayer::registerScannedDevice(std::shared_ptr<BleDeviceHandler> device)
{
    if (!device)
    {
        std::cerr << "Device is null" << std::endl;
        return;
    }

    if (!mScanManager)
    {
        std::cerr << "Scan manager is not initialized" << std::endl;
        return;
    }

    mScanManager->setDevice(device);
}

void BleAbstructLayer::registerConnectDevice(std::shared_ptr<BleDeviceHandler> device)
{
    if (!device)
    {
        std::cerr << "Device is null" << std::endl;
        return;
    }

    if (!mConnectionManager)
    {
        std::cerr << "Connection manager is not initialized" << std::endl;
        return;
    }

    mConnectionManager->setDevice(device);
}

void BleAbstructLayer::connectDevices()
{
    if (!mConnectionManager)
    {
        std::cerr << "Connection manager is not initialized" << std::endl;
    }
    return mConnectionManager->connect();
}

void BleAbstructLayer::sendBleCommand(const BleCommand& command)
{
    if (!mConnectionManager)
    {
        std::cerr << "Connection manager is not initialized" << std::endl;
        return;
    }

    mConnectionManager->sendCommand(command);
}
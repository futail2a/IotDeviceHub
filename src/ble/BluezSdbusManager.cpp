#include "BluezSdbusManager.h"
#include <iostream>

bool BluezSdbusManager::init()
{
    try
    {
        mBusConnection = sdbus::createSystemBusConnection();
        mBluezProxy = sdbus::createProxy(*mBusConnection, BLUEZ_SERVICE, HCI0_PATH);
        mBusConnection->enterEventLoopAsync();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;
};

void BluezSdbusManager::terminate()
{
    disconnectDevices();
    mBusConnection.reset();
    mBluezProxy.reset();
};

bool BluezSdbusManager::startScan()
{
    if(!mBluezProxy)
    {
        std::cout << "BluezSdbusManager is not initialized." << std::endl;
        return false;
    }

    try
    {
        auto method = mBluezProxy->createMethodCall(BLUEZ_ADAPTER, "StartDiscovery");
        mBluezProxy->callMethod(method);
        std::cout << "Start BLE scan" << std::endl;
    }
    catch (const sdbus::Error& e)
    {
        std::cerr << "Failed to start discovery: " << e.getName() << " - " << e.getMessage() << std::endl;
        return false;
    }
    return true;
};

bool BluezSdbusManager::stopScan()
{
    if(!mBluezProxy)
    {
        std::cout << "BluezSdbusManager is not initialized." << std::endl;
        return false;
    }

    try {
        auto method = mBluezProxy->createMethodCall(BLUEZ_ADAPTER, "StopDiscovery");
        mBluezProxy->callMethod(method);
        std::cout << "Stop BLE scan" << std::endl;
    }
    catch (const sdbus::Error& e)
    {
        std::cerr << "Failed to stop discovery: " << e.getName() << " - " << e.getMessage() << std::endl;
        return false;
    }
    return true;
};

void BluezSdbusManager::setDevice(std::shared_ptr<BleDeviceHandler> device)
{
    if (device)
    {
        mBleDeviceHandlers.push_back(device);
        std::string devicePath = HCI0_PATH + "/dev_" + convertToBluezMac(device->getMacAddr());
        std::string matchRule = "type='signal',interface='org.freedesktop.DBus.Properties',"
                                "member='PropertiesChanged',path='" + devicePath + "'";
        auto match = mBusConnection->addMatch(matchRule, [this](sdbus::Message& msg) { onPropertiesChanged(msg, this); });
        mMacSlotMap[device->getMacAddr()] = std::move(match);

        mMacProxyMap[device->getMacAddr()] = sdbus::createProxy(*mBusConnection, BLUEZ_SERVICE, devicePath);
    }
    else
    {
        std::cerr << "BleDeviceHandler is null" << std::endl;
    }
};

std::weak_ptr<BleDeviceHandler> BluezSdbusManager::findDeviceFromMac(std::string mac)
{
    for(auto device : mBleDeviceHandlers)
    {
        if(auto dev = device.lock())
        {
            if(dev->getMacAddr() == mac)
            {
                return device;
            }
        }
    }
    return std::weak_ptr<BleDeviceHandler>();
}

void BluezSdbusManager::onPropertiesChanged(sdbus::Message& msg, BluezSdbusManager* handler)
{
    // std::cout << "PropertiesChanged signal received" << std::endl;

    std::string interfaceName;
    msg >> interfaceName;
    if(interfaceName != BLUEZ_DEVICE)
    {
        std::cout << "PropertiesChanged signal for unsupported interface: " << interfaceName << std::endl;
        return;
    }

    std::map<std::string, sdbus::Variant> changedProps;
    std::vector<std::string> invalidated;
    msg >> changedProps >> invalidated;

    auto path = msg.getPath();
    auto it = changedProps.find("Connected");
    if (it != changedProps.end())
    {
        bool connected = it->second.get<bool>();
        std::string mac = convertFromBluezMac(path);
        auto device = handler->findDeviceFromMac(mac);
        if (auto dev = device.lock())
        {
            if (connected)
            {
                dev->setState(BleDeviceState::CONNECTED);
                dev->onConnected();
                std::cout << "Device " << mac << " connected." << std::endl;
            }
            else
            {
                dev->setState(BleDeviceState::DISCONNECTED);
                dev->onDisconnected();
                std::cout << "Device " << mac << " disconnected." << std::endl;
            }
        }
        else
        {
            std::cout << "No registered device found for MAC: " << mac << std::endl;
        }
    }
    else
    {
        // std::cout << "Connected property not found in PropertiesChanged signal." << std::endl;
    }
}

void BluezSdbusManager::connectDevices()
{
    for(auto device : mBleDeviceHandlers)
    {
        if(auto dev = device.lock())
        {
            if(dev->getState() != BleDeviceState::DISCONNECTED)
            {
                if(dev->getConnCnt() >= MAX_CONN_RETRY)
                {
                    std::cout << "Device " << dev->getMacAddr() << " reached max connection attempts, resetting counter." << std::endl;
                    dev->resetConnCnt();
                    dev->setState(BleDeviceState::DISCONNECTED);
                }
                dev->incrementConnCnt();
                continue;
            }

            std::string devicePath = HCI0_PATH + "/dev_" + convertToBluezMac(dev->getMacAddr());
            std::cout << "Trying to connect device " << devicePath <<  std::endl;

            try {
                dev->setState(BleDeviceState::CONNECTING);
                auto invoker = mMacProxyMap[dev->getMacAddr()]->callMethodAsync("Connect")
                    .onInterface(BLUEZ_DEVICE)
                    .uponReplyInvoke([dev, this](const sdbus::Error* error){
                        if (error)
                        {
                            std::cerr << "Async connect failed: " << error->getName() << " - " << error->getMessage() << std::endl;
                        }
                        else
                        {
                            std::cout << "Async connect succeeded." << std::endl;
                        }
                });
            }
            catch (const sdbus::Error& e)
            {
                std::cerr << "Failed to connect to device " << devicePath << ": "
                          << e.getName() << " - " << e.getMessage() << std::endl;
            }
        }
        else
        {
            continue;
        }
    }
};

void BluezSdbusManager::disconnectDevices()
{
    for(auto device : mBleDeviceHandlers)
    {
        if(auto dev = device.lock())
        {
            if(dev->getState() != BleDeviceState::CONNECTED)
            {
                std::cout << "Device " << dev->getMacAddr() << " is not in CONNECTED state: " << static_cast<int>(dev->getState()) << ", skipping disconnection attempt." << std::endl;
                continue;
            }

            std::string devicePath = HCI0_PATH + "/dev_" + convertToBluezMac(dev->getMacAddr());
            std::cout << "Trying to disconnect device " << devicePath <<  std::endl;

            try {
                mMacProxyMap[dev->getMacAddr()]->callMethod("Disconnect") //TODO, to be async?
                    .onInterface(BLUEZ_DEVICE);
                std::cout << "Disconnection request sent for " << dev->getMacAddr() << std::endl;
                dev->setState(BleDeviceState::DISCONNECTING);
            }
            catch (const sdbus::Error& e)
            {
                std::cerr << "Failed to disconnect device " << dev->getMacAddr() << ": "
                          << e.getName() << " - " << e.getMessage() << std::endl;
            }
        }
        else
        {
            continue;
        }
    }
}

void BluezSdbusManager::sendCommand(const BleCommand& command)
{
    auto device = findDeviceFromMac(command.macAddr);
    if (auto dev = device.lock())
    {
        if (dev->getState() != BleDeviceState::CONNECTED)
        {
            std::cout << "Device " << dev->getMacAddr() << " is not in CONNECTED state: "
                      << static_cast<int>(dev->getState()) << ", skipping command send." << std::endl;
            return;
        }

        std::string devicePath = HCI0_PATH + "/dev_" + convertToBluezMac(dev->getMacAddr());
        std::string fullPath = devicePath + command.charPath;

        auto charProxy = sdbus::createProxy(*mBusConnection, BLUEZ_SERVICE, fullPath);
        try {
            std::map<std::string, sdbus::Variant> sdbusOptions;
            for(const auto& [key, value] : command.options)
            {
                sdbusOptions[key] = sdbus::Variant(value);
            }

            if (command.method == "WriteValue")
            {
                charProxy->callMethod("WriteValue")
                    .onInterface(GATT_CHAR_1)
                    .withArguments(command.data, sdbusOptions);
                std::cout << "WriteValue command sent to " << dev->getMacAddr() << std::endl;
            }
            else if (command.method == "ReadValue")
            {
                auto invoker = charProxy->callMethod("ReadValue");
                invoker.onInterface(GATT_CHAR_1).withArguments(sdbusOptions);
                std::vector<uint8_t> value;
                invoker.storeResultsTo(value);
                std::cout << "ReadValue command sent to " << dev->getMacAddr() << ", received "
                          << value.size() << " bytes." << std::endl;
            }
            else
            {
                std::cerr << "Unknown method: " << command.method << std::endl;
            }
        }
        catch (const sdbus::Error& e)
        {
            std::cerr << "Failed to send command to device " << dev->getMacAddr() << ": "
                      << e.getName() << " - " << e.getMessage() << std::endl;
        }
    }
    else
    {
        std::cerr << "Device with MAC " << command.macAddr << " not found" << std::endl;
    }
};

// void BluezSdbusManager::onInterfacesAdded(sdbus::Signal& signal)
// {
//     sdbus::ObjectPath objectPath;
//     std::map<std::string, std::map<std::string, sdbus::Variant>> interfaces;

//     signal >> objectPath >> interfaces;

//     auto it = interfaces.find("org.bluez.Device1");
//     if (it == interfaces.end())
//     {
//         std::cout << "InterfacesAdded signal does not contain Device1 interface." << std::endl;
//         return;
//     }

//     const auto& props = it->second;
//     auto addrIt = props.find("Address");
//     if (addrIt == props.end())
//     {
//         std::cout << "Device1 interface does not contain Address property." << std::endl;
//         return;
//     }

//     std::string address = addrIt->second.get<std::string>();

//     for(auto device : mBleDeviceHandlers)
//     {
//         if(auto dev = device.lock())
//         {
//             std::string targetMac = dev->getMacAddr();
//             if(address == targetMac)
//             {
//                 // std::cout << "Found interested device: " << address << std::endl;
//                 const auto& props = it->second;
//                 auto advDataIt = props.find("ServiceData");
//                 if (advDataIt != props.end())
//                 {
//                     const auto& variant = advDataIt->second;
//                     const auto& serviceData = variant.get<std::map<std::string, std::vector<uint8_t>>>();

//                     for (const auto& [uuid, data] : serviceData) {
//                         dev->onAdvPacketRecived(data);
//                     }
//                 }
//             }
//         }
//         else
//         {
//             continue;
//         }
//     }
// };
#include "BluezSdbusManager.h"
#include <iostream>

bool BluezSdbusManager::init()
{
    try
    {
        mConnection = sdbus::createSystemBusConnection();
        mBluezProxy = sdbus::createProxy(*mConnection, BLUEZ_SERVICE, HCI0_PATH);
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
    mConnection.reset();
    mBluezProxy.reset();
};

bool BluezSdbusManager::startScan()
{
    if(!mConnection || !mBluezProxy)
    {
        std::cout << "BluezSdbusManager is not initialized." << std::endl;
        return false;
    }

    try {
        mBluezProxy->callMethod("StartDiscovery").onInterface(BLUEZ_ADAPTER);
        mConnection->enterEventLoopAsync();
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
    if(!mConnection || !mBluezProxy)
    {
        std::cout << "BluezSdbusManager is not initialized." << std::endl;
        return false;
    }

    try {
        auto method = mBluezProxy->createMethodCall("StopDiscovery", BLUEZ_ADAPTER.c_str());
        mBluezProxy->callMethod(method);
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
    }
    else
    {
        std::cerr << "BleDeviceHandler is null" << std::endl;
    }

    if(mBluezProxy)
    {
        mBluezProxy->registerSignalHandler(
            "org.freedesktop.DBus.ObjectManager",
            "InterfacesAdded",
            [this](sdbus::Signal& signal) { onInterfacesAdded(signal); }
            );
    }
    if(mConnection)
    {
        mBluezProxy->registerSignalHandler(
            "org.freedesktop.DBus.Properties",
            "PropertiesChanged",
            [this](sdbus::Signal& signal) { onPropertiesChanged(signal, this); }
        );
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

void BluezSdbusManager::onPropertiesChanged(sdbus::Signal& signal, BluezSdbusManager* handler)
{
    std::string interfaceName;
    std::map<std::string, sdbus::Variant> changedProps;
    std::vector<std::string> invalidated;

    signal >> interfaceName >> changedProps >> invalidated;

    if (interfaceName != BLUEZ_DEVICE)
    {
        std::cout << "PropertiesChanged signal for unsupported interface: " << interfaceName << std::endl;
        return;
    }

    auto path = signal.getPath();

    auto it = changedProps.find("Connected");
    if (it != changedProps.end()) {
        bool connected = it->second.get<bool>();
        std::string mac = convertFromBluezMac(path);

        auto device = handler->findDeviceFromMac(mac);
        if (auto dev = device.lock()) {
            if (connected) {
                dev->setState(BleDeviceState::CONNECTED);
                dev->onConnected();
                std::cout << "Device " << mac << " connected." << std::endl;
            } else {
                dev->setState(BleDeviceState::DISCONNECTED);
                dev->onDisconnected();
                std::cout << "Device " << mac << " disconnected." << std::endl;
            }
        }
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
                // std::cout << "Device " << dev->getMacAddr() << " is not in DISCONNECTED state: " << static_cast<int>(dev->getState()) << ", skipping connection attempt." << std::endl;
                continue;
            }

            std::string devicePath = HCI0_PATH + "/dev_" + convertToBluezMac(dev->getMacAddr());

            auto deviceProxy = sdbus::createProxy(*mConnection, BLUEZ_SERVICE, devicePath);
            try {
                deviceProxy->callMethod("Connect").onInterface(BLUEZ_DEVICE);
                std::cout << "Connection request sent for " << dev->getMacAddr() << std::endl;
                dev->setState(BleDeviceState::CONNECTING);
            }
            catch (const sdbus::Error& e)
            {
                std::cerr << "Failed to connect to device " << dev->getMacAddr() << ": "
                          << e.getName() << " - " << e.getMessage() << std::endl;
            }
        }
        else
        {
            continue;
        }
    }
};

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

        auto charProxy = sdbus::createProxy(*mConnection, BLUEZ_SERVICE, fullPath);
        try {
            if (command.method == "WriteValue")
            {
                charProxy->callMethod("WriteValue")
                    .onInterface(GATT_CHAR_1)
                    .withArguments(command.data, command.options);
                std::cout << "WriteValue command sent to " << dev->getMacAddr() << std::endl;
            }
            else if (command.method == "ReadValue")
            {
                auto invoker = charProxy->callMethod("ReadValue");
                invoker.onInterface(GATT_CHAR_1).withArguments(command.options);
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

void BluezSdbusManager::onInterfacesAdded(sdbus::Signal& signal)
{
        sdbus::ObjectPath objectPath;
        std::map<std::string, std::map<std::string, sdbus::Variant>> interfaces;

        signal >> objectPath >> interfaces;

        auto it = interfaces.find("org.bluez.Device1");
        if (it == interfaces.end())
        {
            std::cout << "InterfacesAdded signal does not contain Device1 interface." << std::endl;
            return;
        }

        const auto& props = it->second;
        auto addrIt = props.find("Address");
        if (addrIt == props.end())
        {
            std::cout << "Device1 interface does not contain Address property." << std::endl;
            return;
        }

        std::string address = addrIt->second.get<std::string>();

        for(auto device : mBleDeviceHandlers)
        {
            if(auto dev = device.lock())
            {
                std::string targetMac = dev->getMacAddr();
                if(address == targetMac)
                {
                    // std::cout << "Found interested device: " << address << std::endl;
                    const auto& props = it->second;
                    auto advDataIt = props.find("ServiceData");
                    if (advDataIt != props.end())
                    {
                        const auto& variant = advDataIt->second;
                        const auto& serviceData = variant.get<std::map<std::string, std::vector<uint8_t>>>();

                        for (const auto& [uuid, data] : serviceData) {
                            dev->onAdvPacketRecived(data);
                        }
                    }
                }
            }
            else
            {
                continue;
            }
        }
};
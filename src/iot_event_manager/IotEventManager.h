#ifndef IOT_EVENT_MANAGER_H
#define IOT_EVENT_MANAGER_H

#include "IIotEventManager.h"
#include <unordered_map>
#include <functional>
#include <string>

class IotEventManager : public IIotEventManager
{
public:
    IotEventManager() = default;
    ~IotEventManager() = default;

    void onEvent(const std::string& eventName, const std::string& eventData) override;
    void registerEventHandler(const std::string& eventName, std::function<void(const std::string&)> handler) override;
    void unregisterEventHandler(const std::string& eventName) override;

private:
    std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> handlers;
};

#endif
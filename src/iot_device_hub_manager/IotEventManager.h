#ifndef IOT_EVENT_MANAGER_IMPL_H
#define IOT_EVENT_MANAGER_IMPL_H

#include <unordered_map>
#include <functional>
#include <string>

class IotEventManager
{
public:
    IotEventManager() = default;
    ~IotEventManager() = default;

    void onEvent(const std::string& eventName, const std::string& eventData);
    void registerEventHandler(const std::string& eventName, std::function<void(const std::string&)> handler);
    void unregisterEventHandler(const std::string& eventName);

private:
    std::unordered_map<std::string, std::vector<std::function<void(const std::string&)>>> handlers;
};

#endif
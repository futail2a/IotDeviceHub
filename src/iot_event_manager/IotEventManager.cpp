#include "IotEventManager.h"

void IotEventManager::onEvent(const std::string& eventName, const std::string& eventData)
{
    auto it = handlers.find(eventName);
    if (it != handlers.end())
    {
        for (auto& h : it->second) {
            h(eventData);
        }
    }
}

void IotEventManager::registerEventHandler(const std::string& eventName, std::function<void(const std::string&)> handler)
{
    handlers[eventName].push_back(handler);
}

void IotEventManager::unregisterEventHandler(const std::string& eventName)
{
    handlers.erase(eventName);
}

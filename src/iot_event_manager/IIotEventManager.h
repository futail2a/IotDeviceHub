#ifndef I_IOT_EVENT_MANAGER_H
#define I_IOT_EVENT_MANAGER_H

#include <functional>
#include <string>

class IIotEventManager
{
public:
    IIotEventManager() = default;
    virtual ~IIotEventManager(){};

    virtual void onEvent(const std::string& eventName, const std::string& eventData) = 0;
    virtual void registerEventHandler(const std::string& eventName, std::function<void(const std::string&)> handler) = 0;
    virtual void unregisterEventHandler(const std::string& eventName) = 0;
};

#endif
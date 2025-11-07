#ifndef IOT_EVENT_MANAGER_MOCK_H
#define IOT_EVENT_MANAGER_MOCK_H

#include "IIotEventManager.h"
#include <gmock/gmock.h>

class IotEventManager : public IIotEventManager
{
public:
    MOCK_METHOD(void, onEvent, (const std::string& eventName, const std::string& eventData), (override));
    MOCK_METHOD(void, registerEventHandler,
                (const std::string& eventName, std::function<void(const std::string&)> handler), (override));
    MOCK_METHOD(void, unregisterEventHandler, (const std::string& eventName), (override));
};

#endif
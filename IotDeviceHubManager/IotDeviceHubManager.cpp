#include "IotDeviceHubManager.h"
#include "BluezAbstructLayer.h"
#include <iostream>
#include <iomanip>

void IotDeviceHubManager::addObserver(const std::shared_ptr<SensorObserver> observer)
{
    observers.push_back(observer);
}

void IotDeviceHubManager::removeObserver(const std::shared_ptr<SensorObserver> observer)
{
    for(auto it = observers.begin();it!=observers.end();++it)
    {
        if(*it == observer)
        {
            observers.erase(it);
            return;
        }
    }
    std::cout << "not found"<<std::endl;
}

void IotDeviceHubManager::notify(const std::vector<uint8_t>& data)
{
    for(auto observer : observers)
    {
        observer->update(data);
    }
}

void IotDeviceHubManager::run()
{
  std::vector<uint8_t> adv_data;

  m_bluez->init();
  m_bluez->start_scan();
  sleep(10); //wait to scan devices

  //Todo: Implement loop logic
  for(int i = 0; i < 5; i++)
  {
    adv_data = m_bluez->get_adv_data();
    notify(adv_data);
    adv_data.clear();
    sleep(1);
  }

  m_bluez->stop_scan();
}

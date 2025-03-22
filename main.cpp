#include "IotDeviceHubManager.h"

int main() {
  IotDeviceHubManager manager;
  if(manager.init())
  {
    manager.run();
  }
  return 0;
}

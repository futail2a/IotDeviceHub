#include "IotDeviceHubManager.h"

int main() {
  IotDeviceHubManager manager;
  if(manager.init())
  {
    manager.run();
  }
  manager.stop();
  return 0;
}

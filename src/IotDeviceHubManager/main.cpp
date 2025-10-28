#include "IotDeviceHubManager.h"
#include <csignal>

static IotDeviceHubManager* g_manager = nullptr;

void signal_handler(int signum) {
    if (g_manager) {
        std::cout << "Signal " << signum << " received, stopping..." << std::endl;
        g_manager->terminate();
    }
    exit(0);
}

int main() {
  IotDeviceHubManager manager;
  g_manager = &manager;
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

    if (!manager.init())
    {
        std::cerr << "Failed to initialize IotDeviceHubManager" << std::endl;
        return 1;
    }
    manager.run();
    return 0;
}

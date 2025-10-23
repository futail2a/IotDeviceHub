#include <iostream>
#include <unistd.h>
#include <mosquitto.h>

#include "MqttManager.h"

int main() {
  MqttManager mqtt_manager;

  if (!mqtt_manager.init("iot_device_client"))
  {
    std::cerr << "Failed to initialize MQTT manager" << std::endl;
    return -1;
  }

  mqtt_manager.start();

  mqtt_manager.subscribe("iot_device_hub/sensor_data/bed_room/temperature");
  mqtt_manager.subscribe("iot_device_hub/sensor_data/bed_room/humidity");
  mqtt_manager.subscribe("iot_device_hub/sensor_data/entrance/pir_utc");
  mqtt_manager.subscribe("iot_device_hub/sensor_data/entrance/light_intensity");

  while (true)
  {
    sleep(1);
  }
  mqtt_manager.stop();
  mqtt_manager.deinit();
  return 0;
}

#ifndef MQTT_TOPIC_LIST_H
#define MQTT_TOPIC_LIST_H

#include <string>

// e.g. "iot_device_hub/sensor_data/bed_room/temperature"
const std::string IOT_TOPIC_SENS_DATA_BASE = "iot_device_hub/sensor_data";
const std::string IOT_TOPIC_SENS_DATA_TEMP = "/temperature";
const std::string IOT_TOPIC_SENS_DATA_HUMID = "/humidity";

const std::string IOT_TOPIC_SENS_DATA_PIR_UTC = "/pir_utc";
const std::string IOT_TOPIC_SENS_DATA_LIGHT_INTENSITY = "/light_intensity";
#endif
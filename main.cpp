#include "BluezAbstructLayer.h"
#include <iostream>
#include "SwtichBotApiDataParser.h"
#include <iomanip>

constexpr uint8_t SERVICEDATA_LEN = 6;
constexpr uint8_t BIT_7_MASK = 0x80;
constexpr uint8_t BIT_0_6_MASK = 0x7f;

void print_temp_humid(const std::vector<uint8_t>& data)
{
  if(data.size() > SERVICEDATA_LEN)
  {
    std::cerr << "Service Data length is longer than " << SERVICEDATA_LEN << "bytes" << std::endl;
    return;
  }

  uint8_t temp = data[4]&BIT_0_6_MASK;
  std::cout << "Temperature: " << ((data[4]&BIT_7_MASK) ? "" : "-") << (int)temp << "°C" << std::endl;

  // Notice: API document explains Bit[7] is Templature Scale and Bit[6:0] is Humidity Value
  // But as far as I checked with actual value from device, whole Bit[7:0] expresse humidity value, and Bit[7] is not temperature scale
  // https://github.com/OpenWonderLabs/SwitchBotAPI-BLE/blob/latest/devicetypes/meter.md#broadcast-mode
  uint8_t humid = data[5];
  std::cout << "Humidity: " << (int)humid << "%" << std::endl;
}

int main() {

    BluezAbstructLayer bluez(new WoSensorTHDataParser());
    bluez.init();
    bluez.start_scan();
    sleep(10); //wait to scan devices
    print_temp_humid(bluez.get_adv_data());
    bluez.stop_scan();

    return 0;
}

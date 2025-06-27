#ifndef Bluez_SOCK_LAYER_H
#define Bluez_SOCK_LAYER_H

#include <string>
#include <vector>
#include <unistd.h>
#include <memory>

#include "BluetoothAbstructLayer.h"
#include "SensorDataHandler.h"

constexpr uint8_t MAC_LENGTH = 6;
constexpr uint8_t SERVICE_DATA_TYPE = 0x16;
constexpr uint8_t SERVICE_DATA_INDEX = 4;

class BluezSockLayer : public BluetoothAbstructLayer
{
public:
    BluezSockLayer();
    ~BluezSockLayer();

    bool init() override;
    void add_sensor_data_handler(std::shared_ptr<SensorDataHandler> sensorDataHandler) override;
    bool start_scan() override;
    bool stop_scan() override;
    void check_adv_data() override;
    void check_connectable();

private:
    std::vector<std::pair<std::shared_ptr<SensorDataHandler>,std::string>> m_sensorDataHandlerMacPairs; // pari of handler and converted mac address
    int m_sock=-1;

    void m_print_byte_array(const std::vector<uint8_t>& data);
    std::string to_lower_mac(const uint8_t* addr);

    const uint8_t type = 0x01; // LE scan
    const uint16_t interval = 0x0010; // 10 ms
    const uint16_t window = 0x0010; // 10 ms
    const uint8_t own_type = 0x00; // Public address
    const uint8_t filter = 0x00; // No filter
    const int to = 1000; // Timeout in milliseconds

};

#endif
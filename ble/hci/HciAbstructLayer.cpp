#include "HciAbstructLayer.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

HciAbstructLayer::HciAbstructLayer()
{
}

HciAbstructLayer::~HciAbstructLayer()
{
}

bool HciAbstructLayer::init()
{
    int device_id = hci_get_route(nullptr);
    if (device_id < 0)
    {
        std::cerr << "Failed to get HCI root: " << device_id << std::endl;
        return false;
    }

    m_sock = hci_open_dev(device_id);
    if (m_sock < 0)
    {
        std::cerr << "Failed to open HCI device: " << device_id << std::endl;
        return false;
    }

    if (ioctl(m_sock, HCIDEVUP, device_id) < 0)
    {
        if (errno == EALREADY)
        {
            std::cout << "HCI device hci"<<device_id<< "is already up"<<std::endl;
        }
        else
        {
            std::cerr << "ioctl(HCIDEVUP) failed: " << strerror(errno) << std::endl;
            close(m_sock);
            return false;
        }
    }
    else
    {
        std::cout << "HCI device hci<<"<<device_id<< "brought up successfully"<<std::endl;
    }

    return true;
}

void HciAbstructLayer::add_sensor_data_handler(std::shared_ptr<SensorDataHandler> sensorDataHandler)
{
    if (sensorDataHandler)
    {
        std::string normalized_handler_mac;
        for (char c : sensorDataHandler->get_device_mac())
        {
            if (c != ':')
            {
                normalized_handler_mac += std::tolower(static_cast<unsigned char>(c));
            }
        }
        m_sensorDataHandlerMacPairs.push_back({sensorDataHandler,normalized_handler_mac});
    }
    else
    {
        std::cerr << "Sensor data handler is null" << std::endl;
    }
}

bool HciAbstructLayer::start_scan()
{
    struct hci_filter new_filter;
    hci_filter_clear(&new_filter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);

    if (setsockopt(m_sock, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0)
    {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    if (hci_le_set_scan_parameters(m_sock, type, interval, window, own_type, filter, to) < 0)
    {
        std::cerr << "hci_le_set_scan_parameters failed: " << strerror(errno) << std::endl;
        return false;
    }

    if (hci_le_set_scan_enable(m_sock, 0x01, 0x00, 1000) < 0)
    {
        std::cerr << "hci_le_set_scan_enable failed: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Scanning for BLE advertisements..." << std::endl;

    return true;
}

bool HciAbstructLayer::stop_scan()
{
    hci_le_set_scan_enable(m_sock, 0x00, 0x00, 1000);
    close(m_sock);
    std::cout << "Scan stopped" << std::endl;
    return true;
}

std::string HciAbstructLayer::to_lower_mac(const uint8_t* addr) {
    std::ostringstream oss;
    for (int i = MAC_LENGTH-1; i >= 0; --i)
    {
        oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(addr[i]);
    }
    return oss.str();
}

void HciAbstructLayer::check_adv_data()
{
    uint8_t buf[HCI_MAX_EVENT_SIZE];
    int len = read(m_sock, buf, sizeof(buf));
    if (len < 0)
    {
        std::cerr << "Failed to read HCI event: " << strerror(errno) << std::endl;
        return;
    }

    evt_le_meta_event* meta = (evt_le_meta_event*)(buf + (1 + HCI_EVENT_HDR_SIZE));
    if (meta->subevent != EVT_LE_ADVERTISING_REPORT)
    {
        std::cerr << "Unexpected event type: " << static_cast<int>(meta->subevent) << std::endl;
        return;
    }

    le_advertising_info* info = (le_advertising_info*)(meta->data + 1);
    std::string mac = to_lower_mac(info->bdaddr.b);

    for(auto handler : m_sensorDataHandlerMacPairs)
    {
        if (mac == handler.second) {
            std::cout << "Found MAC: " << mac << std::endl;
            int i = 0;
            while (i < info->length) {
                uint8_t field_len = info->data[i];
                if (field_len == 0 || i + field_len >= info->length)
                {
                    std::cerr << "Invalid field length or index out of bounds" << std::endl;
                    break;
                }
                uint8_t ad_type = info->data[i + 1];
                if (ad_type == SERVICE_DATA_TYPE)
                {
                    std::cout << "Service Data: ";
                    for (int j = i+SERVICE_DATA_INDEX; j < i+field_len+1; ++j)
                    {
                        std::cout << std::hex << std::setfill('0') << std::setw(2)
                                    << static_cast<int>(info->data[j]) << " ";
                    }
                    std::cout << std::dec << std::endl;
                    std::vector<uint8_t> service_data(info->data + i + SERVICE_DATA_INDEX, info->data+i+field_len+1);
                    handler.first->update(service_data);
                    break;
                }
                i += field_len + 1;
            }
        }
    }
}

#include "BleSockScanManager.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <unistd.h>

BleSockScanManager::~BleSockScanManager()
{
    isScanning = false;
    if (mScanningThread.joinable())
    {
        mScanningThread.join();
    }
}

bool BleSockScanManager::init()
{
    int device_id = hci_get_route(nullptr);
    if (device_id < 0)
    {
        std::cerr << "Failed to get HCI root: " << device_id << std::endl;
        return false;
    }

    mSock = hci_open_dev(device_id);
    if (mSock < 0)
    {
        std::cerr << "Failed to open HCI device: " << device_id << std::endl;
        return false;
    }

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = SOCK_TIMEOUT_USEC;
    setsockopt(mSock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    if (ioctl(mSock, HCIDEVUP, device_id) < 0)
    {
        if (errno == EALREADY)
        {
            std::cout << "HCI device hci"<<device_id<< "is already up"<<std::endl;
        }
        else
        {
            std::cerr << "ioctl(HCIDEVUP) failed: " << strerror(errno) << std::endl;
            close(mSock);
            return false;
        }
    }
    else
    {
        std::cout << "HCI device hci<<"<<device_id<< "brought up successfully"<<std::endl;
    }

    return true;
}

void BleSockScanManager::terminate()
{
    if(isScanning)
    {
        stopScan();
    }

    if (mSock >= 0)
    {
        close(mSock);
        mSock = -1;
    }
}

void BleSockScanManager::setDevice(std::shared_ptr<BleDeviceHandler> device)
{
    if (device)
    {
        std::string normalizedMac;
        for (char c : device->getMacAddr())
        {
            if (c != ':')
            {
                normalizedMac += std::tolower(static_cast<unsigned char>(c));
            }
        }
            mBleDeviceHandlerMacPairs.push_back({device,normalizedMac});
        }
    else
    {
        std::cerr << "BleDeviceHandler is null" << std::endl;
    }
}

bool BleSockScanManager::startScan()
{
    struct hci_filter new_filter;
    hci_filter_clear(&new_filter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);

    if (setsockopt(mSock, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0)
    {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    if (hci_le_set_scan_parameters(mSock, type, interval, window, own_type, filter, to) < 0)
    {
        std::cerr << "hci_le_set_scan_parameters failed: " << strerror(errno) << std::endl;
        return false;
    }

    if (hci_le_set_scan_enable(mSock, 0x01, 0x00, 1000) < 0)
    {
        std::cerr << "hci_le_set_scan_enable failed: " << strerror(errno) << std::endl;
        return false;
    }

    std::cout << "Scanning for BLE advertisements..." << std::endl;

    isScanning = true;
    mScanningThread = std::thread(&BleSockScanManager::scanning, this);
    return true;
}

bool BleSockScanManager::stopScan()
{
    hci_le_set_scan_enable(mSock, 0x00, 0x00, 1000);
    std::cout << "Scan stopped" << std::endl;

    isScanning = false;
    if (mScanningThread.joinable())
    {
        mScanningThread.join();
    }

    return true;
}

std::string BleSockScanManager::toLowerMac(const uint8_t* addr) {
    std::ostringstream oss;
    for (int i = MAC_LENGTH-1; i >= 0; --i)
    {
        oss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(addr[i]);
    }
    return oss.str();
}

void BleSockScanManager::scanning()
{
    while(isScanning)
    {
        uint8_t buf[HCI_MAX_EVENT_SIZE];
        int len = read(mSock, buf, sizeof(buf));
        if (len < 0)
        {
            std::cerr << "Failed to read HCI event: " << strerror(errno) << std::endl;
            return;
        }

        evt_le_meta_event* meta = (evt_le_meta_event*)(buf + (1 + HCI_EVENT_HDR_SIZE));
        //TODO: Assess which event to be unexpected event need to exit process
        if (meta->subevent != EVT_LE_ADVERTISING_REPORT && meta->subevent != EVT_LE_CONN_COMPLETE)
        {
            std::cerr << "Unexpected event type: " << static_cast<int>(meta->subevent) << std::endl;
            continue;
        }

        le_advertising_info* info = (le_advertising_info*)(meta->data + 1);
        std::string mac = toLowerMac(info->bdaddr.b);

        for(auto handler : mBleDeviceHandlerMacPairs)
        {
            if (mac == handler.second) {
                // std::cout << "Found MAC: " << mac << std::endl;
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
                        // Printing received service data
                        // std::cout << "Service Data: ";
                        // for (int j = i+SERVICE_DATA_INDEX; j < i+field_len+1; ++j)
                        // {
                        //     std::cout << std::hex << std::setfill('0') << std::setw(2)
                        //                 << static_cast<int>(info->data[j]) << " ";
                        // }
                        // std::cout << std::dec << std::endl;

                        std::vector<uint8_t> service_data(info->data + i + SERVICE_DATA_INDEX, info->data+i+field_len+1);
                        handler.first->onAdvPacketRecived(service_data);
                        break;
                    }
                    i += field_len + 1;
                }
            }
        }
    }
    std::cout << "Exit from scanning" << std::endl;

}

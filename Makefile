CC  := g++

CFLAGS := `pkg-config --cflags --libs dbus-1 libmosquitto `
INCLUDES := -I./include -I./ble -I./IotDeviceHubManager -I./mqtt -I./devices -I/usr/local/include/Poco
CFLAGS += $(INCLUDES)
LDFLAGS := -lbluetooth -lPocoFoundation -lPocoJSON -lPocoUtil -lstdc++fs -Wl,-rpath,/usr/local/lib
CFLAGS += -std=c++17 -Wall -O3

DEFAULT_TARGET := IotDeviceHub
DEFAULT_SRCS   := $(shell find . -name '*.cpp' -not -path './client/*' )
DEFAULT_SRCS += IotDeviceHubManager/main.cpp

CLIENT_TARGET := IotDeviceClient
CLIENT_SRCS   := $(shell find . -name '*.cpp' -not -path './IotDeviceHubManager/*' )
CLIENT_SRCS += client/main.cpp

BASE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
CONFIG_PATH := $(abspath $(MAKEFILE_DIR)/config/config.json)
CFLAGS += -D'CONFIG_FILE_PATH="$(CONFIG_PATH)"'

all: $(DEFAULT_TARGET)

$(DEFAULT_TARGET): $(DEFAULT_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

install: $(DEFAULT_TARGET)
	install -D -m 755 $(DEFAULT_TARGET) /usr/local/bin/$(DEFAULT_TARGET)
	install -D -m 644 iotdevicehub.service /etc/systemd/system/iotdevicehub.service

clean:
	-rm -f $(DEFAULT_TARGET) $(CLIENT_TARGET)

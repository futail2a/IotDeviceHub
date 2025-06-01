CC  := g++

CFLAGS := `pkg-config --cflags --libs dbus-1 libmosquitto `
INCLUDES := -I./include -I./ble/bluez -I./ble/hci -I./IotDeviceHubManager -I./mqtt -I./devices
CFLAGS += $(INCLUDES)
LDFLAGS := -lbluetooth

DEFAULT_TARGET := IotDeviceHub
DEFAULT_SRCS   := $(shell find . -name '*.cpp' -not -path './client/*' )
DEFAULT_SRCS += IotDeviceHubManager/main.cpp

CLIENT_TARGET := IotDeviceClient
CLIENT_SRCS   := $(shell find . -name '*.cpp' -not -path './IotDeviceHubManager/*' )
CLIENT_SRCS += client/main.cpp

all: $(DEFAULT_TARGET)

$(DEFAULT_TARGET): $(DEFAULT_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	-rm -f $(DEFAULT_TARGET) $(CLIENT_TARGET)

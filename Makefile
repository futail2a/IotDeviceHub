CC  := g++

CFLAGS := `pkg-config --cflags --libs dbus-1 libmosquitto`
INCLUDES := -I./include -I./ble -I./IotDeviceHubManager -I./mqtt
CFLAGS += $(INCLUDES)

DEFAULT_TARGET := IotDeviceHub
DEFAULT_SRCS   := $(shell find . -name '*.cpp' -not -path './client/*' )
DEFAULT_SRCS += IotDeviceHubManager/main.cpp

CLIENT_TARGET := IotDeviceClient
CLIENT_SRCS   := $(shell find . -name '*.cpp' -not -path './IotDeviceHubManager/*' )
CLIENT_SRCS += client/main.cpp

all: clean $(DEFAULT_TARGET)

$(DEFAULT_TARGET): $(DEFAULT_SRCS)
	$(CC) -o $@ $^ $(CFLAGS)

$(CLIENT_TARGET): $(CLIENT_SRCS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	-rm -f $(DEFAULT_TARGET) $(CLIENT_TARGET)

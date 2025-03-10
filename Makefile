CC  := g++

CFLAGS := `pkg-config --cflags --libs dbus-1`
TARGET := IotDeviceHub
INCLUDES := -I./include -I./BluezAbstructLayer -I./IotDeviceHubManager -I./MqttAbstructLayer
CFLAGS += $(INCLUDES)
SRCS   := $(shell find . -name '*.cpp')

$(TARGET): $(SRCS)
	$(CC) -o $@ $^ $(CFLAGS)

all: clean $(TARGET)

clean:
	-rm -f $(TARGET)

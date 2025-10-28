CC  := g++

CFLAGS := `pkg-config --cflags --libs dbus-1 libmosquitto `
INCLUDES := -I./include -I./ble -I./IotDeviceHubManager -I./mqtt -I./devices -I/usr/local/include/Poco
CFLAGS += $(INCLUDES)
LDFLAGS := -lbluetooth -lPocoFoundation -lPocoJSON -lPocoUtil -lstdc++fs -Wl,-rpath,/usr/local/lib
CFLAGS += -std=c++17 -Wall -O3

TARGET := IotDeviceHub
EXCLUDE_DIR := tests/
TARGET_SRCS   := $(shell find . -name '*.cpp' ! -path "$(EXCLUDE_DIR)/*")
TARGET_SRCS := $(filter-out tests/*.cpp, $(SRC_FILES))
TARGET_SRCS += IotDeviceHubManager/main.cpp

all: $(TARGET)

$(TARGET): $(TARGET_SRCS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

install: $(TARGET)
	install -D -m 755 $(TARGET) /usr/local/bin/$(TARGET)
	install -D -m 644 iotdevicehub.service /etc/systemd/system/iotdevicehub.service

clean:
	-rm -f $(TARGET)

uninstall:
	-rm -f /usr/local/bin/$(TARGET)
	-rm -f /etc/systemd/system/iotdevicehub.service
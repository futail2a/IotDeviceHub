CC  := g++

CFLAGS := `pkg-config --cflags --libs dbus-1`
TARGET := IotDeviceHub
SRCS   :=  main.cpp BluezAbstructLayer.cpp SwtichBotApiDataParser.cpp

$(TARGET): $(SRCS)
	$(CC) -o $@ $^ $(CFLAGS)

all: clean $(TARGET)

clean:
	-rm -f $(TARGET)

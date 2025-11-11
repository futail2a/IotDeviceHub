cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

find_package(Poco REQUIRED Foundation JSON Util)

file(GLOB_RECURSE ALL_CPP RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS *.cpp)
set(TARGET_SRCS "")
foreach(_src ${ALL_CPP})
  if(_src MATCHES "^(build|CMakeFiles|\.git|\.vs|tests)/")
    continue()
  endif()
  list(APPEND TARGET_SRCS ${CMAKE_SOURCE_DIR}/${_src})
endforeach()

include_directories(
  ${CMAKE_SOURCE_DIR}/src/include
  ${CMAKE_SOURCE_DIR}/src/ble
  ${CMAKE_SOURCE_DIR}/src/iot_device_hub_manager
  ${CMAKE_SOURCE_DIR}/src/iot_event_manager
  ${CMAKE_SOURCE_DIR}/src/mqtt
  ${CMAKE_SOURCE_DIR}/src/devices
  /usr/local/include/Poco
  ${DBUS_INCLUDE_DIRS}
  ${MOSQUITTO_INCLUDE_DIRS}
)

add_executable(IotDeviceHub ${TARGET_SRCS})
target_compile_features(IotDeviceHub PRIVATE cxx_std_17)
target_compile_options(IotDeviceHub PRIVATE -O3 -Wall)
target_link_libraries(IotDeviceHub PRIVATE
  ${DBUS_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
  Poco::Foundation
  Poco::JSON
  Poco::Util
  bluetooth
)

find_library(STDCXXFS_LIB NAMES stdc++fs)
if(STDCXXFS_LIB)
  target_link_libraries(IotDeviceHub PRIVATE ${STDCXXFS_LIB})
endif()

install(TARGETS IotDeviceHub RUNTIME DESTINATION /usr/local/bin)
install(FILES ${CMAKE_SOURCE_DIR}/iotdevicehub.service DESTINATION /etc/systemd/system PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(FILES ${CMAKE_SOURCE_DIR}/src/config/config.json DESTINATION /etc/iotdevicehub PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

if(NOT DEFINED ENV{GITHUB_ACTIONS})
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/config)
file(COPY ${CMAKE_SOURCE_DIR}/src/config/config.json
     DESTINATION ${CMAKE_BINARY_DIR}/bin/config)
endif()

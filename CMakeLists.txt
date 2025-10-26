cmake_minimum_required(VERSION 3.14)
project(IotDeviceHub LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

find_package(Poco REQUIRED Foundation JSON Util)

file(GLOB_RECURSE ALL_CPP RELATIVE ${CMAKE_SOURCE_DIR} CONFIGURE_DEPENDS *.cpp)
set(DEFAULT_SRCS "")
set(CLIENT_SRCS "")
foreach(_src ${ALL_CPP})
  if(_src MATCHES "^(build|CMakeFiles|\.git|\.vs|tests)/")
    continue()
  endif()

  if(_src MATCHES "^client/")
    list(APPEND CLIENT_SRCS ${CMAKE_SOURCE_DIR}/${_src})
  else()
    list(APPEND DEFAULT_SRCS ${CMAKE_SOURCE_DIR}/${_src})
  endif()
endforeach()

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/ble
  ${CMAKE_SOURCE_DIR}/IotDeviceHubManager
  ${CMAKE_SOURCE_DIR}/mqtt
  ${CMAKE_SOURCE_DIR}/devices
  /usr/local/include/Poco
  ${DBUS_INCLUDE_DIRS}
  ${MOSQUITTO_INCLUDE_DIRS}
)

add_executable(IotDeviceHub ${DEFAULT_SRCS})
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

option(BUILD_CLIENT "Build IotDeviceClient" OFF)
if(BUILD_CLIENT)
  if(CLIENT_SRCS)
    add_executable(IotDeviceClient ${CLIENT_SRCS})
    target_compile_features(IotDeviceClient PRIVATE cxx_std_17)
    target_compile_options(IotDeviceClient PRIVATE -O3 -Wall)
    target_link_libraries(IotDeviceClient PRIVATE
      ${DBUS_LIBRARIES}
      ${MOSQUITTO_LIBRARIES}
      Poco::Foundation
      Poco::JSON
      Poco::Util
      bluetooth
    )
    if(STDCXXFS_LIB)
      target_link_libraries(IotDeviceClient PRIVATE ${STDCXXFS_LIB})
    endif()
  else()
    message(WARNING "BUILD_CLIENT set but no client sources found under ${CMAKE_SOURCE_DIR}/client")
  endif()
endif()

install(TARGETS IotDeviceHub RUNTIME DESTINATION /usr/local/bin)
install(FILES ${CMAKE_SOURCE_DIR}/iotdevicehub.service DESTINATION /etc/systemd/system PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
  add_subdirectory(tests)
endif()
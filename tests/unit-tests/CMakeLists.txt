cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

set(GTEST_INCLUDE_PATH /usr/local/include/gtest)
set(GMOCK_INCLUDE_PATH /usr/local/include/gmock)

set(TEST_SOURCES
  Test_WoBulbHandler.cpp
)

file(GLOB TAEGET_SOURCE
  ${PROJECT_SOURCE_DIR}/src/devices/WoBulbHandler.cpp
  ${PROJECT_SOURCE_DIR}/src/IotDeviceHubManager/IotEventManager.cpp
)

find_package(Threads REQUIRED)
find_package(Poco REQUIRED Foundation JSON Util)

add_executable(unit_tests ${TEST_SOURCES})

target_sources(unit_tests
    PRIVATE
    ${TAEGET_SOURCE}
)
# target_compile_options(unit_tests PRIVATE -O0 -g --coverage)
# target_link_options   (unit_tests PRIVATE --coverage)

target_include_directories(unit_tests
    PUBLIC
    ${GTEST_INCLUDE_PATH}
    ${GMOCK_INCLUDE_PATH}
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/IotDeviceHubManager
    ${CMAKE_SOURCE_DIR}/src/devices
)

target_link_libraries(unit_tests
  PRIVATE
  gtest
  gtest_main
)

target_link_libraries(unit_tests
  PRIVATE
  ${DBUS_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
  Poco::Foundation
  Poco::JSON
  Poco::Util
  bluetooth
  Threads::Threads
)

add_custom_target(
  run-test
  COMMAND ${CMAKE_BINARY_DIR}/bin/unit_tests
)

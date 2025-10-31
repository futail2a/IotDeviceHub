find_package(PkgConfig REQUIRED)
pkg_check_modules(DBUS REQUIRED dbus-1)
find_package(Threads REQUIRED)
find_package(Poco REQUIRED Foundation JSON Util)

set(TEST_SOURCES
    Test_WoBulbHandler.cpp
    Test_WoHandHandler.cpp
)

file(GLOB TAEGET_SOURCE
    ${PROJECT_SOURCE_DIR}/src/devices/WoBulbHandler.cpp
    ${PROJECT_SOURCE_DIR}/src/devices/WoHandHandler.cpp
    ${PROJECT_SOURCE_DIR}/src/devices/WoMotionSensorHandler.cpp
    ${PROJECT_SOURCE_DIR}/src/iot_device_hub_manager/IotEventManager.cpp
)

add_executable(ut_devices ${TEST_SOURCES})

target_sources(ut_devices
    PRIVATE
    ${TAEGET_SOURCE}
)

target_include_directories(ut_devices
    PUBLIC
    ${GTEST_INCLUDE_PATH}
    ${GMOCK_INCLUDE_PATH}
    ${DBUS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/iot_device_hub_manager
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ble
    ${CMAKE_SOURCE_DIR}/src/mqtt
)

target_link_libraries(ut_devices
    PRIVATE
    gtest
    gtest_main
)

target_link_libraries(ut_devices
    PRIVATE
    ${DBUS_LIBRARIES}
    ${MOSQUITTO_LIBRARIES}
    Poco::Foundation
    Poco::JSON
    Poco::Util
    bluetooth
    Threads::Threads
)

set_target_properties(ut_devices PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
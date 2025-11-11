find_package(Poco REQUIRED Foundation JSON Util)

set(TEST_SOURCES
    Test_IotDeviceHubManager.cpp
)

file(GLOB TAEGET_SOURCE
    ${PROJECT_SOURCE_DIR}/src/iot_device_hub_manager/IotDeviceHubManager.cpp
    ${PROJECT_SOURCE_DIR}/src/devices/WoBulbHandler.cpp
    ${PROJECT_SOURCE_DIR}/src/devices/WoHandHandler.cpp
    # ${PROJECT_SOURCE_DIR}/src/devices/WoMotionSensorHandler.cpp
)

add_executable(ut_iot_device_hub_manager ${TEST_SOURCES})

target_sources(ut_iot_device_hub_manager
    PRIVATE
    ${TAEGET_SOURCE}
)

target_include_directories(ut_iot_device_hub_manager
    PUBLIC
    ${GTEST_INCLUDE_PATH}
    ${GMOCK_INCLUDE_PATH}
    ${DBUS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/iot_device_hub_manager
    ${CMAKE_SOURCE_DIR}/src/iot_event_manager
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ble
    ${CMAKE_SOURCE_DIR}/src/mqtt
    ${CMAKE_SOURCE_DIR}/tests/mocks
)

target_link_libraries(ut_iot_device_hub_manager
    PRIVATE
    gtest
    gtest_main
    gmock
    gmock_main
)

set_target_properties(ut_iot_device_hub_manager PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
find_package(Poco REQUIRED Foundation JSON Util)

set(TEST_SOURCES
    Test_MqttManager.cpp
)

file(GLOB TAEGET_SOURCE
    ${PROJECT_SOURCE_DIR}/src/mqtt/MqttManager.cpp
)

add_executable(ut_mqtt ${TEST_SOURCES})

target_sources(ut_mqtt
    PRIVATE
    ${TAEGET_SOURCE}
)

target_include_directories(ut_mqtt
    PUBLIC
    ${GTEST_INCLUDE_PATH}
    ${GMOCK_INCLUDE_PATH}
    ${DBUS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/include
    ${CMAKE_SOURCE_DIR}/src/iot_event_manager
    ${CMAKE_SOURCE_DIR}/src/devices
    ${CMAKE_SOURCE_DIR}/src/ble
    ${CMAKE_SOURCE_DIR}/src/mqtt
    ${CMAKE_CURRENT_SOURCE_DIR}/mocks
)

target_link_libraries(ut_mqtt
    PRIVATE
    gtest
    gtest_main
)

target_link_libraries(ut_mqtt
    PRIVATE
    ${DBUS_LIBRARIES}
    ${MOSQUITTO_LIBRARIES}
)

set_target_properties(ut_mqtt PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
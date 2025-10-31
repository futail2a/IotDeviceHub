cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

set(GTEST_INCLUDE_PATH /usr/local/include/gtest)
set(GMOCK_INCLUDE_PATH /usr/local/include/gmock)

set(TEST_SOURCES
    Test_WoBulbHandler.cpp
    Test_WoHandHandler.cpp
)

# add_subdirectory(ble)
add_subdirectory(devices)
# add_subdirectory(iot_device_hub_manager)
add_subdirectory(mqtt)

add_custom_target(test
    DEPENDS ut_devices ut_mqtt #ut_iot_device_hub_manager
    COMMAND ${CMAKE_BINARY_DIR}/bin/ut_devices
    COMMAND ${CMAKE_BINARY_DIR}/bin/ut_mqtt
    # COMMAND ${CMAKE_BINARY_DIR}/bin/ut_iot_device_hub_manager
)

find_program(LCOV_EXECUTABLE lcov)
find_program(GENHTML_EXECUTABLE genhtml)

add_custom_target(coverage
    DEPENDS test
    COMMAND ${LCOV_EXECUTABLE} -d . -c -o coverage.info
    COMMAND ${LCOV_EXECUTABLE} -r coverage.info "*/googletest/*" "*/tests/*" "*/c++/*" -o coverageFiltered.info
    COMMAND ${GENHTML_EXECUTABLE} -o lcovHtml --num-spaces 4 -s --legend coverageFiltered.info
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
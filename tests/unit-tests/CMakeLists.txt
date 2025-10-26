cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

set(GTEST_INCLUDE_PATH /usr/local/include/gtest)
set(GMOCK_INCLUDE_PATH /usr/local/include/gmock)

set(TEST_SOURCES
  Test_WoBulbHandler.cpp
)

file(GLOB TAEGET_SOURCE
  ${PROJECT_SOURCE_DIR}/devices/WoBulbHandler.cpp
  ${PROJECT_SOURCE_DIR}/IotDeviceHubManager/IotEventManager.cpp
)

find_package(Threads REQUIRED)

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
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/IotDeviceHubManager
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
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/unit_tests
)

# target_compile_options(unit_tests PRIVATE -O0 -g --coverage)
# target_link_options(unit_tests PRIVATE --coverage)
# target_link_libraries(unit_tests GTest::gtest_main)

# include(GoogleTest)
# gtest_discover_tests(unit_tests)

# add_test(NAME unit_tests COMMAND unit_tests)


# # Option to enable coverage build and a convenience target to produce HTML report via lcov/genhtml
# option(ENABLE_COVERAGE "Enable generation of lcov code coverage report (requires lcov/genhtml)" OFF)
# if(ENABLE_COVERAGE)
#   if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#     message(STATUS "Coverage enabled: adding --coverage flags")
#     # Use flags that generate gcov/lcov data. Prefer target-level flags in larger projects;
#     # for simplicity apply globally here so tests and main get instrumented.
#     add_compile_options(-O0 -g --coverage)
#     add_link_options(--coverage)

#     find_program(LCOV_EXEC lcov)
#     find_program(GENHTML_EXEC genhtml)
#     if(NOT LCOV_EXEC)
#       message(WARNING "lcov not found. Install lcov (e.g. apt install lcov) to use coverage target.")
#     endif()
#     if(NOT GENHTML_EXEC)
#       message(WARNING "genhtml not found. Install lcov (provides genhtml) to use coverage target.")
#     endif()

#     if(LCOV_EXEC AND GENHTML_EXEC)
#       add_custom_target(coverage
#         COMMAND ${CMAKE_COMMAND} -E echo "Cleaning previous coverage data"
#         COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --zerocounters
#         COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --capture --initial --output-file ${CMAKE_BINARY_DIR}/coverage_base.info
#         COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -j ${CMAKE_BUILD_PARALLEL_LEVEL}
#         COMMAND ${LCOV_EXEC} --directory ${CMAKE_BINARY_DIR} --capture --output-file ${CMAKE_BINARY_DIR}/coverage.info
#         COMMAND ${LCOV_EXEC} --add-tracefile ${CMAKE_BINARY_DIR}/coverage_base.info --add-tracefile ${CMAKE_BINARY_DIR}/coverage.info --output-file ${CMAKE_BINARY_DIR}/coverage_total.info
#         # remove system and external deps from report - adjust patterns as needed
#         COMMAND ${LCOV_EXEC} --remove ${CMAKE_BINARY_DIR}/coverage_total.info '/usr/*' '*/_deps/*' '*/external/*' --output-file ${CMAKE_BINARY_DIR}/coverage_filtered.info
#         COMMAND ${GENHTML_EXEC} -o ${CMAKE_BINARY_DIR}/coverage ${CMAKE_BINARY_DIR}/coverage_filtered.info
#         WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#         COMMENT "Running tests and generating lcov HTML report in ${CMAKE_BINARY_DIR}/coverage"
#         VERBATIM
#       )
#     endif()
#   else()
#     message(WARNING "ENABLE_COVERAGE set but compiler is not GCC/Clang; coverage target disabled.")
#   endif()
# endif()
add_library(MosquittoMock STATIC
    MosquittoMock.cpp
)

target_include_directories(MosquittoMock PUBLIC
    ${GTEST_INCLUDE_PATH}
    ${GMOCK_INCLUDE_PATH}
    ${CMAKE_SOURCE_DIR}/tests/mocks
    /usr/include/
)

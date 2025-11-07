add_library(MosquittoMock STATIC
    MosquittoMock.cpp
)

target_include_directories(MosquittoMock PUBLIC
    /usr/include/
)

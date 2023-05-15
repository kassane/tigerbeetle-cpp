cmake_minimum_required(VERSION 3.14)

project(tb_cpp VERSION 1.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/deps.cmake)

find_package(Threads REQUIRED)
if(WIN32)
    link_libraries(ws2_32 wmsock)
endif()

include_directories(include)

add_executable(${PROJECT_NAME}
    include/tb_client.hpp
    src/main.cpp
)
target_link_libraries(${PROJECT_NAME}
    PUBLIC "${CMAKE_SOURCE_DIR}/vendor/lib/libtb_client.so" fmt
    PRIVATE $<TARGET_NAME_IF_EXISTS:Threads::Threads>
)
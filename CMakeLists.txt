cmake_minimum_required(VERSION 3.14)

project(tb_cpp VERSION 0.10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/deps.cmake)
include(cmake/findTigerbeetle.cmake)
find_package(Threads REQUIRED)
find_library(TigerBeetle PATH ${TIGERBEETLE_LIBRARY_DIR})
if(WIN32)
    link_libraries(ws2_32 wmsock)
endif()

include_directories(include)
include_directories(${TIGERBEETLE_INCLUDE_DIR})

add_executable(${PROJECT_NAME}
    "include/tb_client.hpp"
    "src/main.cpp"
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -fno-sanitize=all)
target_link_libraries(${PROJECT_NAME}
    PUBLIC TigerBeetle
    PRIVATE $<TARGET_NAME_IF_EXISTS:Threads::Threads> fmt
)
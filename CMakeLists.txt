cmake_minimum_required(VERSION 3.14)

project(tb_cpp VERSION 0.10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RUN_INSTALL_ZIG "Run install_zig.sh script" ON)
option(BUILD_TB_C_CLIENT "Build c_client libraries with Zig" ON)
option(RUN_TB_SERVER "Run Tigerbeetle server" OFF)
option(RUN_TB_TEST "Run Tigerbeetle test" OFF)

include(cmake/deps.cmake)
include(cmake/findTigerbeetle.cmake)
find_package(Threads REQUIRED)
if(WIN32)
    link_libraries(ws2_32)
endif()

include_directories(include)
include_directories(${TIGERBEETLE_INCLUDE_DIR})

add_executable(${PROJECT_NAME}
    "include/tb_client.hpp"
    "src/main.cpp"
)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -fexperimental-library -fno-sanitize=all)
target_link_libraries(${PROJECT_NAME}
    PUBLIC TigerBeetle
    PRIVATE $<TARGET_NAME_IF_EXISTS:Threads::Threads> fmt
)
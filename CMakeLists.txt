cmake_minimum_required(VERSION 3.14)

project(tb_cpp VERSION 0.10 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RUN_INSTALL_ZIG "Run zig install 0.9.1" ON)
option(BUILD_TB_C_CLIENT "Build c_client library with Zig" ON)
option(TIGERBEETLE_BUILD_SHARED_LIBS "Build TigerBeetle as a shared library" OFF)
option(RUN_TB_TEST "Run Tigerbeetle test" OFF)
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)

include(cmake/deps.cmake)
include(cmake/FindTigerBeetle.cmake)
find_package(Threads REQUIRED)
if(WIN32)
    link_libraries(ws2_32)
endif()

add_executable(${PROJECT_NAME}
    "example/basic.cpp"
)
target_include_directories(${PROJECT_NAME}
    PUBLIC
    "include"
    ${TIGERBEETLE_INCLUDE_DIR}
)
if(ENABLE_ASAN)
	target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
	target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
endif()
if(ENABLE_TSAN)
	target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
	target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=thread)
endif()
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wshadow -Wpedantic)
target_link_libraries(${PROJECT_NAME}
    PUBLIC TigerBeetle
    PRIVATE $<TARGET_NAME_IF_EXISTS:Threads::Threads> fmt
)

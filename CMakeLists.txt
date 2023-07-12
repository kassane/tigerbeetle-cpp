cmake_minimum_required(VERSION 3.14)

project(tb_cpp VERSION 0.10 LANGUAGES CXX)

cmake_policy(SET CMP0111 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RUN_INSTALL_ZIG "Run zig install 0.9.1" ON)
option(BUILD_TB_C_CLIENT "Build c_client library with Zig" ON)
option(TIGERBEETLE_BUILD_SHARED_LIBS "Build TigerBeetle as a shared library" OFF)
option(RUN_TB_TEST "Run Tigerbeetle test" OFF)
option(USE_FMT "Build with Fmt logger" ON)
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)

include(cmake/deps.cmake)
include(cmake/FindTigerBeetle.cmake)
find_package(Threads REQUIRED)

get_git_version(GIT_VERSION)
get_git_authors(GIT_AUTHORS)
get_date(CURRENT_DATE)

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

if(WIN32)
    link_libraries(ntdll ws2_32 wsock32 mswsock)
endif()

if(USE_FMT)
    add_compile_definitions(-DUSE_FMT)
    target_link_libraries(${PROJECT_NAME}
        PUBLIC TigerBeetle::TigerBeetle
        PRIVATE Threads::Threads fmt
    )
else()
    target_link_libraries(${PROJECT_NAME}
        PUBLIC TigerBeetle::TigerBeetle
        PRIVATE Threads::Threads
    )
endif()

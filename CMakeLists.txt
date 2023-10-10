cmake_minimum_required(VERSION 3.8...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.14)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(TigerbeetleCpp VERSION 0.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose default type of build (Debug)" FORCE)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
elseif(NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    set(CMAKE_CONFIGURATION_TYPES "Release")
endif()
option(RUN_INSTALL_ZIG "Run zig install (build tigerbeeble library)" ON)
option(BUILD_TB_C_CLIENT "Build c_client library with Zig" ON)
option(BUILD_EXAMPLES "Build client examples" OFF)
option(BUILD_TESTS "Build tests" ON)
option(TIGERBEETLE_BUILD_SHARED_LIBS "Build TigerBeetle as a shared library" OFF)
option(RUN_TB_TEST "Run Tigerbeetle test" OFF)
option(USE_FMT "Build with Fmt logger" OFF)
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)

if(BUILD_EXAMPLES)
    # Define the list of target names
    set(APP_TARGETS
        basic
        two_phase
        two_phase_many
    )
endif()

include(FeatureSummary)
include(cmake/deps.cmake)
if(BUILD_TESTS)
    set(APP_TARGETS accountTest transferTest)
endif()
include(cmake/FindTigerBeetle.cmake)

if(WIN32)
    set(WIN_LIBS ntdll ws2_32 wsock32 mswsock)
else()
    set(WIN_LIBS "")
endif()
target_include_directories(TigerBeetle INTERFACE "include" ${TIGERBEETLE_INCLUDE_DIR})
if(USE_FMT)
    add_compile_definitions(-DUSE_FMT)
endif()
if(BUILD_EXAMPLES)
    find_package(Threads REQUIRED)

    foreach(app ${APP_TARGETS})
        # Add the source file for each target
        add_executable(${app} "examples/${app}.cpp")

        # Set common compile options for all targets
        target_compile_options(${app} PRIVATE
            -Wall -Wextra -Werror -Wshadow -Wpedantic
        )

        if(ENABLE_ASAN)
            target_compile_options(${app} PRIVATE -fsanitize=address -fsanitize=undefined)
            target_link_options(${app} PRIVATE -fsanitize=address -fsanitize=undefined)
        endif()

        if(ENABLE_TSAN)
            target_compile_options(${app} PRIVATE -fsanitize=thread)
            target_link_options(${app} PRIVATE -fsanitize=thread)
        endif()

        if(USE_FMT)
            target_link_libraries(${app}
                PUBLIC TigerBeetle::TigerBeetle
                PRIVATE Threads::Threads fmt::fmt ${WIN_LIBS}
            )
        else()
            target_link_libraries(${app}
                PUBLIC TigerBeetle::TigerBeetle
                PRIVATE Threads::Threads ${WIN_LIBS}
            )
        endif()
    endforeach()
endif()

if(BUILD_TESTS)
    find_package(Threads REQUIRED)
    foreach(app ${APP_TARGETS})
        # Add the source file for each target
        add_executable(${app} "tests/${app}.cpp")

        # Set common compile options for all targets
        target_compile_options(${app} PRIVATE
            -Wall -Wextra -Werror -Wshadow -Wpedantic
        )

        if(ENABLE_ASAN)
            target_compile_options(${app} PRIVATE -fsanitize=address -fsanitize=undefined)
            target_link_options(${app} PRIVATE -fsanitize=address -fsanitize=undefined)
        endif()

        if(ENABLE_TSAN)
            target_compile_options(${app} PRIVATE -fsanitize=thread)
            target_link_options(${app} PRIVATE -fsanitize=thread)
        endif()

        target_link_libraries(${app}
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads ${WIN_LIBS} doctest::doctest
        )
    endforeach()
endif()

file(
    WRITE "${CMAKE_BINARY_DIR}/.clang-tidy"
    "
---
Checks: '-*,llvm-twine-local'
...
"
)
feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

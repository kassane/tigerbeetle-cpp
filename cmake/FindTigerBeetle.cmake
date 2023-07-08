# Boost Software License - Version 1.0 - August 17th, 2003

# Created Date: 17 May 2023
# Author: Matheus Catarino França (@kassane) (matheus-catarino@hotmail.com)

# Permission is hereby granted, free of charge, to any person or organization
# obtaining a copy of the software and accompanying documentation covered by
# this license (the "Software") to use, reproduce, display, distribute,
# execute, and transmit the Software, and to prepare derivative works of the
# Software, and to permit third-parties to whom the Software is furnished to
# do so, all subject to the following:

# The copyright notices in the Software and this entire statement, including
# the above license grant, this restriction and the following disclaimer,
# must be included in all copies of the Software, in whole or in part, and
# all derivative works of the Software, unless such copies or derivative
# works are solely in the form of machine-executable object code generated by
# a source language processor.

include(CheckLibraryExists)
if(CMAKE_C_COMPILER_LOADED)
    include(CheckIncludeFile)
    include(CheckCSourceCompiles)
elseif(CMAKE_CXX_COMPILER_LOADED)
    include(CheckIncludeFileCXX)
    include(CheckCXXSourceCompiles)
else()
    message(FATAL_ERROR "FindTigerBeetle only works if either C or CXX language is enabled")
endif()


# Set the root directory of the TigerBeetle library
set(TIGERBEETLE_ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/tb-src)

# Specify the directories for different platforms
set(TIGERBEETLE_INCLUDE_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/include)

if(RUN_INSTALL_ZIG)
    if(NOT EXISTS "${TIGERBEETLE_ROOT_DIR}/zig/zig")
        if(WIN32)
            # Run install_zig.bat script
            execute_process(
                COMMAND cmd /c ${TIGERBEETLE_ROOT_DIR}/scripts/install_zig.bat
                WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
                RESULT_VARIABLE INSTALL_ZIG_RESULT
            )
        else()
            # Run install_zig.sh script
            execute_process(
                COMMAND sh ${TIGERBEETLE_ROOT_DIR}/scripts/install_zig.sh
                WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
                RESULT_VARIABLE INSTALL_ZIG_RESULT
            )
            if(NOT ${INSTALL_ZIG_RESULT} EQUAL 0)
                message(FATAL_ERROR "Failed to run zig install script.")
            endif()
        endif()
    else()
        message(STATUS "Zig already downloaded. Skipping zig install.")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ZIG_BUILD_TYPE "-Drelease-safe=false")
    set(ZIG_CONFIG "-Dconfig=default")
else()
    set(ZIG_BUILD_TYPE "-Drelease-safe")
    set(ZIG_CONFIG "-Dconfig=production")
endif()

if(WIN32)
    set(BUILD_TB ${TIGERBEETLE_ROOT_DIR}/scripts/build.bat)
    set(RUN_WITH_TB ${CMAKE_SOURCE_DIR}/scripts/runner.bat)
else()
    set(BUILD_TB ${TIGERBEETLE_ROOT_DIR}/scripts/build.sh)
    set(RUN_WITH_TB ${CMAKE_SOURCE_DIR}/scripts/runner.sh)
endif()

if(BUILD_TB_C_CLIENT)
    # Build c_client with Zig
    message(STATUS "Build c_client libraries with Zig 0.9.1")
    execute_process(
        COMMAND ${BUILD_TB} c_client ${ZIG_BUILD_TYPE} ${ZIG_CONFIG}
        WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
        RESULT_VARIABLE BUILD_C_CLIENT_RESULT
    )
    if(NOT ${BUILD_C_CLIENT_RESULT} EQUAL 0)
        message(FATAL_ERROR "Failed to build c_client libraries with Zig 0.9.1")
    endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/aarch64-linux-gnu)
    else()
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/x86_64-linux-gnu)
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/aarch64-macos)
    else()
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/x86_64-macos)
    endif()
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/aarch64-windows)
    else()
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/x86_64-windows)
    endif()
endif()

# Set the library type based on the option
if(TIGERBEETLE_BUILD_SHARED_LIBS)
    set(CMAKE_TIGERBEETLE_LIBS_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    if(WIN32)
        # zig don't generate .a to windows, only .lib (mingw/msvc)
        set(CMAKE_TIGERBEETLE_LIBS_SUFFIX ".lib")
    else()
        set(CMAKE_TIGERBEETLE_LIBS_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
endif()

if(WIN32)
    set(CMAKE_TIGERBEETLE_LIBS_INIT "tb_client")
else()
    set(CMAKE_TIGERBEETLE_LIBS_INIT "libtb_client")
endif()
find_library(TigerBeetle NAMES ${CMAKE_TIGERBEETLE_LIBS_INIT}${CMAKE_TIGERBEETLE_LIBS_SUFFIX} PATHS ${TIGERBEETLE_LIBRARY_DIR})

if (TigerBeetle STREQUAL "TigerBeetle-NOTFOUND")
    message(FATAL_ERROR "Failed to find TigerBeetle library.")
endif()

if(TIGERBEETLE_BUILD_SHARED_LIBS)
    add_library(TigerBeetle SHARED IMPORTED)
    file(COPY ${TIGERBEETLE_LIBRARY_DIR}/${CMAKE_TIGERBEETLE_LIBS_INIT}${CMAKE_TIGERBEETLE_LIBS_SUFFIX}
        DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    add_library(TigerBeetle STATIC IMPORTED)
endif()
set_target_properties(TigerBeetle PROPERTIES IMPORTED_LOCATION ${TigerBeetle})
set_target_properties(TigerBeetle PROPERTIES IMPORTED_IMPLIB ${TIGERBEETLE_LIBRARY_DIR}/${CMAKE_TIGERBEETLE_LIBS_INIT}${CMAKE_TIGERBEETLE_LIBS_SUFFIX})


if(RUN_TB_TEST)
    # Build and run test with Zig
    message(STATUS "Build and run TigerBeetle tests with Zig")
    execute_process(
        COMMAND ${BUILD_TB} test ${ZIG_BUILD_TYPE}
        WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
        RESULT_VARIABLE BUILD_RUN_WITH_TB_RESULT
    )
    if(NOT ${BUILD_RUN_WITH_TB_RESULT} EQUAL 0)
        message(FATAL_ERROR "Failed to build and run test with Zig.")
    endif()
endif()

# Create a custom target to run_with_tb
add_custom_target(run_with_tb
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
)

# # Add a post-build event to kill the tigerbeetle start process after running ${PROJECT_NAME}
add_custom_command(TARGET run_with_tb POST_BUILD
    COMMAND ${RUN_WITH_TB}
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "Killing tigerbeetle start process..."
    COMMAND ${CMAKE_COMMAND} -E sleep 2 # Delay to ensure ${PROJECT_NAME} has started
    COMMAND ${CMAKE_COMMAND} -E cmake_echo_color --cyan "Terminating tigerbeetle start process..."
    WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
    COMMENT "Running ${PROJECT_NAME} with TigerBeetle"
)

# Clean the zig-cache directory
execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TIGERBEETLE_ROOT_DIR}/zig-cache
    RESULT_VARIABLE CLEAN_ZIG_CACHE_RESULT
)

if(NOT ${CLEAN_ZIG_CACHE_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to clean zig-cache directory.")
endif()

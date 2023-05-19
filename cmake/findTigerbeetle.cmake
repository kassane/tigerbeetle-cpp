include(CheckLibraryExists)
if(CMAKE_C_COMPILER_LOADED)
    include (CheckIncludeFile)
    include (CheckCSourceCompiles)
elseif(CMAKE_CXX_COMPILER_LOADED)
    include (CheckIncludeFileCXX)
    include (CheckCXXSourceCompiles)
else()
    message(FATAL_ERROR "FindTigerBeetle only works if either C or CXX language is enabled")
endif()


# Set the root directory of the TigerBeetle library
set(TIGERBEETLE_ROOT_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/tb-src)

# Specify the directories for different platforms
set(TIGERBEETLE_INCLUDE_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/include)
set(TIGERBEETLE_LIBRARY_DIR "")
set(CMAKE_TIGERBEETLE_LIBS_INIT "")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/aarch64-linux-musl)
    else()
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/x86_64-linux-gnu)
    endif()
    set(CMAKE_TIGERBEETLE_LIBS_INIT "libtb_client.a")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/aarch64-macos)
    else()
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/x86_64-macos)
    endif()
    set(CMAKE_TIGERBEETLE_LIBS_INIT "libtb_client.a")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/x86_64-windows)
    else()
        set(TIGERBEETLE_LIBRARY_DIR ${TIGERBEETLE_ROOT_DIR}/src/clients/c/lib/aarch64-windows)
    endif()
    set(CMAKE_TIGERBEETLE_LIBS_INIT "tb_client.lib")
endif()

find_library(TigerBeetle PATHS ${TIGERBEETLE_LIBRARY_DIR})

add_library(TigerBeetle STATIC IMPORTED)
set_target_properties(TigerBeetle PROPERTIES IMPORTED_LOCATION ${TIGERBEETLE_LIBRARY_DIR}/${CMAKE_TIGERBEETLE_LIBS_INIT})

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
                message(FATAL_ERROR "Failed to run install_zig script.")
            endif()
        endif()
    else()
        message(STATUS "Zig already downloaded. Skipping install_zig.")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(ZIG_BUILD_TYPE "-Drelease-safe=false")
else()
    set(ZIG_BUILD_TYPE "-Drelease-safe")
endif()

if(BUILD_TB_C_CLIENT)
    # Build c_client with Zig
    execute_process(
    COMMAND ${TIGERBEETLE_ROOT_DIR}/zig/zig build c_client ${ZIG_BUILD_TYPE}
    WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
    RESULT_VARIABLE BUILD_C_CLIENT_RESULT
    )
    if(NOT ${BUILD_C_CLIENT_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to build c_client with Zig.")
    endif()
endif()

if(RUN_TB_TEST)
    # Build and run test with Zig
    execute_process(
    COMMAND ${TIGERBEETLE_ROOT_DIR}/zig/zig build test ${ZIG_BUILD_TYPE}
    WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
    RESULT_VARIABLE BUILD_RUN_WITH_TB_RESULT
    )
    if(NOT ${BUILD_RUN_WITH_TB_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to build and run test with Zig.")
    endif()
endif()

if(RUN_TB_SERVER)
    # Run tigerbeetle format command in the background
    execute_process(
        COMMAND ${TIGERBEETLE_ROOT_DIR}/zig-out/bin/tigerbeetle format --cluster=0 --replica=0 --replica-count=1 0_0.tigerbeetle
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_BACKGROUND
    )

    # Run tigerbeetle start command in the background
    execute_process(
        COMMAND ${TIGERBEETLE_ROOT_DIR}/zig-out/bin/tigerbeetle start --addresses=0.0.0.0:3000
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND_BACKGROUND
        OUTPUT_VARIABLE TIGERBEETLE_START_PID
    )

    # Kill the tigerbeetle start process if needed
    if(TIGERBEETLE_START_PID)
        if(WIN32)
            execute_process(
            COMMAND taskkill /F /PID ${TIGERBEETLE_START_PID}
            )
        else()
            execute_process(
            COMMAND kill ${TIGERBEETLE_START_PID}
            )
        endif()
    endif()

endif()

# Clean the zig directory
# execute_process(
#     COMMAND ${CMAKE_COMMAND} -E remove_directory ${TIGERBEETLE_ROOT_DIR}/zig
#     RESULT_VARIABLE CLEAN_ZIG_CACHE_RESULT
# )
  
if(NOT ${CLEAN_ZIG_CACHE_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to clean zig directory.")
endif()

# Clean the zig-cache directory
execute_process(
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TIGERBEETLE_ROOT_DIR}/zig-cache
    RESULT_VARIABLE CLEAN_ZIG_CACHE_RESULT
)
  
if(NOT ${CLEAN_ZIG_CACHE_RESULT} EQUAL 0)
    message(FATAL_ERROR "Failed to clean zig-cache directory.")
endif()

# Return to the project root directory
execute_process(
  COMMAND cd ../../../
  WORKING_DIRECTORY ${TIGERBEETLE_ROOT_DIR}
)
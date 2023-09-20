cmake_minimum_required(VERSION 3.14)

project(tb_cpp VERSION 0.3.0 LANGUAGES CXX)

cmake_policy(SET CMP0111 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(RUN_INSTALL_ZIG "Run zig install (build tigerbeeble library)" ON)
option(BUILD_TB_C_CLIENT "Build c_client library with Zig" ON)
option(BUILD_EXAMPLES "Build client examples" OFF)
option(TIGERBEETLE_BUILD_SHARED_LIBS "Build TigerBeetle as a shared library" OFF)
option(RUN_TB_TEST "Run Tigerbeetle test" OFF)
option(USE_FMT "Build with Fmt logger" ON)
option(ENABLE_ASAN "Build with AddressSanitizer" OFF)
option(ENABLE_TSAN "Build with ThreadSanitizer" OFF)

include(cmake/deps.cmake)
include(cmake/FindTigerBeetle.cmake)
find_package(Threads REQUIRED)

if(WIN32)
    set(WIN_LIBS ntdll ws2_32 wsock32 mswsock)
else()
    set(WIN_LIBS "")
endif()
target_include_directories(TigerBeetle INTERFACE "include" ${TIGERBEETLE_INCLUDE_DIR})

if(BUILD_EXAMPLES)
    add_executable(basic_example
        "examples/basic.cpp"
    )
    add_executable(two_phase_example
        "examples/two_phase.cpp"
    )
    add_executable(multiple_two_phase_example
        "examples/two_phase_many.cpp"
    )

    if(ENABLE_ASAN)
        target_compile_options(basic_example PRIVATE -fsanitize=address -fsanitize=undefined)
        target_link_options(basic_example PRIVATE -fsanitize=address -fsanitize=undefined)
        target_compile_options(two_phase_example PRIVATE -fsanitize=address -fsanitize=undefined)
        target_link_options(two_phase_example PRIVATE -fsanitize=address -fsanitize=undefined)
        target_compile_options(multiple_two_phase_example PRIVATE -fsanitize=address -fsanitize=undefined)
        target_link_options(multiple_two_phase_example PRIVATE -fsanitize=address -fsanitize=undefined)
    endif()

    if(ENABLE_TSAN)
        target_compile_options(basic_example PRIVATE -fsanitize=thread)
        target_link_options(basic_example PRIVATE -fsanitize=thread)
        target_compile_options(two_phase_example PRIVATE -fsanitize=thread)
        target_link_options(two_phase_example PRIVATE -fsanitize=thread)
        target_compile_options(multiple_two_phase_example PRIVATE -fsanitize=thread)
        target_link_options(multiple_two_phase_example PRIVATE -fsanitize=thread)
    endif()

    target_compile_options(basic_example PRIVATE -Wall -Wextra -Werror -Wshadow -Wpedantic)
    target_compile_options(two_phase_example PRIVATE -Wall -Wextra -Werror -Wshadow -Wpedantic)
    target_compile_options(multiple_two_phase_example PRIVATE -Wall -Wextra -Werror -Wshadow -Wpedantic)

    if(USE_FMT)
        add_compile_definitions(-DUSE_FMT)
        target_link_libraries(basic_example
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads fmt ${WIN_LIBS}
        )
        target_link_libraries(two_phase_example
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads fmt ${WIN_LIBS}
        )
        target_link_libraries(multiple_two_phase_example
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads fmt ${WIN_LIBS}
        )
    else()
        target_link_libraries(basic_example
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads ${WIN_LIBS}
        )
        target_link_libraries(two_phase_example
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads ${WIN_LIBS}
        )
        target_link_libraries(multiple_two_phase_example
            PUBLIC TigerBeetle::TigerBeetle
            PRIVATE Threads::Threads ${WIN_LIBS}
        )
    endif()
endif()

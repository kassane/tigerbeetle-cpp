include(FetchContent)
message(STATUS "-----------------------------------------")
message(STATUS "ASIO    =>  Downloading")
FetchContent_Declare(
  asio
  GIT_REPOSITORY "https://github.com/chriskohlhoff/asio.git"
  GIT_TAG        asio-1-28-0
)
message(STATUS "FMT     =>  Downloading")
FetchContent_Declare(
    fmt
    GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
    GIT_TAG     10.0.0
)
message(STATUS "TB      =>  Downloading")
FetchContent_Declare(
  tb
  GIT_REPOSITORY "https://github.com/tigerbeetledb/tigerbeetle.git"
  GIT_TAG        0.13.9
)

FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()
set(ASIO_PATH ${asio_SOURCE_DIR})

FetchContent_GetProperties(fmt)
if(NOT fmt_POPULATED)
    FetchContent_Populate(fmt)
endif()
set(FMT_PATH ${fmt_SOURCE_DIR})

FetchContent_GetProperties(tb)
if(NOT tb_POPULATED)
    FetchContent_Populate(tb)
endif()
set(TB_PATH ${tb_SOURCE_DIR})

message(STATUS "-----------------------------------------")
message(STATUS "ASIO    =>  Downloaded")
FetchContent_MakeAvailable(asio)
message(STATUS "FMT     =>  Downloaded")
FetchContent_MakeAvailable(fmt)
message(STATUS "TB      =>  Downloaded")
FetchContent_MakeAvailable(fmt)
message(STATUS "-----------------------------------------")

add_subdirectory(${FMT_PATH} ${CMAKE_CURRENT_BINARY_DIR}/fmt EXCLUDE_FROM_ALL)
include_directories(${FMT_PATH}/include)
include_directories(${ASIO_PATH}/asio/include)
include_directories(${TB_PATH}/src/clients/c)

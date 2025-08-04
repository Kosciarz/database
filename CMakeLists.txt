cmake_minimum_required(VERSION 3.20)

project(database 
    VERSION 0.0.1
    LANGUAGES C
    DESCRIPTION "An SQLite clone"
)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    unity
    URL https://github.com/ThrowTheSwitch/Unity/archive/refs/tags/v2.6.1.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(unity)

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)

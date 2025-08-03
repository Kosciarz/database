cmake_minimum_required(VERSION 3.20)

project(database 
    VERSION 0.0.1
    LANGUAGES C
    DESCRIPTION "An SQLite clone"
)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.c)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -pedantic)

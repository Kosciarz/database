set(SOURCES
    "main.c"
    "input.c"
    "getline.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -pedantic)

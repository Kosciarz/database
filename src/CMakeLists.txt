set(GETLINE
    "third_party/getline/getline.c"
)

set(SOURCES
    "main.c"
    "input.c"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${GETLINE})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -pedantic)

set(SOURCES
    "main.c"
    "input.c"
    "getline.c"
)

add_executable(${PROJECT_NAME} ${SOURCES})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /WA /W4)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -pedantic)
endif()

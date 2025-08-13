set(SOURCES
    "input.c"
    "getline.c"
    "parser.c"
    "table.c"
)

add_library(db_core STATIC ${SOURCES})
target_include_directories(db_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT_NAME} "main.c" ${SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE db_core)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
    TABLE_FILE="${CMAKE_CURRENT_BINARY_DIR}/table.bin"
)

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /WA /W4)
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror -pedantic)
endif()

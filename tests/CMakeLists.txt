add_executable(test_input test_input.c)
target_link_libraries(test_input PRIVATE unity db_core)
add_test(NAME test_input COMMAND test_input)

add_executable(test_getline test_getline.c)
target_link_libraries(test_getline PRIVATE unity db_core)
add_test(NAME test_getline COMMAND test_getline)

add_executable(test_parser test_parser.c)
target_link_libraries(test_parser PRIVATE unity db_core)
add_test(NAME test_parser COMMAND test_parser)

add_executable(test_meta_commands test_meta_commands.c)
target_link_libraries(test_meta_commands PRIVATE unity db_core)
add_test(NAME test_meta_commands COMMAND test_meta_commands)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(NAME test_output COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/test_output.py ${CMAKE_BINARY_DIR}/src/database.exe)

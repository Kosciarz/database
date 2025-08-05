add_executable(test_input "test_input.c")
target_link_libraries(test_input PRIVATE unity db_core)
add_test(NAME test_input COMMAND test_input)

add_executable(test_getline "test_getline.c")
target_link_libraries(test_getline PRIVATE unity db_core)
add_test(NAME test_getline COMMAND test_getline)

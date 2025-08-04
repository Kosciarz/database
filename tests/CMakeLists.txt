add_executable(test_main test_main.c)
target_link_libraries(test_main PRIVATE unity)
add_test(NAME test_main COMMAND test_main)

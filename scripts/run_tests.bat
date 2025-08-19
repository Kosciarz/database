@ECHO OFF

cmake --preset debug

cmake --build build\debug

ctest --test-dir build\debug --output-on-failure --verbose

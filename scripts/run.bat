@ECHO OFF

cmake --preset debug

cmake --build build\debug --target database

.\build\debug\src\database.exe

#ifndef GETLINE_H
#define GETLINE_H

#include <stdio.h>
#include <stddef.h>

#ifdef _WIN32
#include <basetsd.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/types.h>
#endif

ssize_t getline(char** buffer, size_t* n, FILE* stream);

#endif // GETLINE_H

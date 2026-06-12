#pragma once
#define USE_DEBUG_PRINT

#include <stdio.h>
#include <stdarg.h>

#ifdef USE_DEBUG_PRINT
#define DEBUG_PRINT(format, ...) debug_print(format, __VA_ARGS__)
#else
#define DEBUG_PRINT(format, ...) 
#endif

void debug_print(const char *format, ...);

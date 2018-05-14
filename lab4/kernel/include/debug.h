#ifndef __DEBUG_H__
#define __DEBUG_H__

//#include <am.h>
#include "klib.h"

//#define LOG
#define DEBUG
// #define TRACE

// Log
#ifdef LOG
  #define Log(format, ...) \
    do { \
      printf("\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
        __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
    } while (0)
#else 
  #define Log(format, ...) ((void)0)
#endif

// Assert
#ifdef DEBUG
  #define Assert(cond) \
    do { \
      if (!(cond)) { \
        printf("\33[1;31m" "Assertion fail at %s:%d" "\33[0m\n", \
          __FILE__, __LINE__); \
        assert(0);\
      } \
    } while (0)
#else
  #define Assert(ignore) ((void)0)
#endif

// // Fence
// #define FENCEBYTE  0xfd
// #define FENCESIZE  32
// void fence_check(uint8_t *fence);
// void fence_set(uint8_t *target);

// Trace
#ifdef TRACE
  #define TRACE_ENTRY \
    printf("\33[1;32m" "[trace] %s: entry" "\33[0m\n" , __func__)
  #define TRACE_EXIT \
    printf("\33[1;32m" "[trace] %s: exit" "\33[0m\n" , __func__)
#else
  #define TRACE_ENTRY ((void)0)
  #define TRACE_EXIT ((void)0)
#endif

// Panic
#define Panic(format, ...) \
  do { \
    printf("\33[1;31m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
    assert(0);\
  } while (0)
  
#endif

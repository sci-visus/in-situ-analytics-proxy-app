#ifndef TALASSCONFIG_H
#define TALASSCONFIG_H

#if defined(WIN32) || defined(WIN64)

typedef  __int64  int64_t;
#ifndef int32_t
  typedef int       int32_t;
#endif
typedef short     int16_t;
typedef char      int8_t;

typedef unsigned __int64  uint64_t;
typedef unsigned long     uint32_t;
typedef unsigned short    uint16_t;
typedef unsigned char     uint8_t;

#else /* unix -- assume we have stdint.h */
     
#include <stdint.h>

#endif

#include <cassert>
#include <cstdio>

typedef @FUNCTION_TYPE@ FunctionType;

typedef @GLOBAL_INDEX_TYPE@ GlobalIndexType;
static const GlobalIndexType GNULL = (GlobalIndexType)-1;

typedef @LOCAL_INDEX_TYPE@ LocalIndexType;
static const LocalIndexType LNULL = (LocalIndexType)-1;

#if defined(WIN32) || defined(WIN64)

#define stwarning(msg,...) {;}
#define sterror(condition,msg,...) {;}
#define stmessage(condition,msg,...) {;}

#else

#ifndef NDEBUG

#include <cstdio>
#include <cstring>

#define stwarning(msg,...) {char error[200] = "WARNING: %s::%u:\n\t";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);}
#define sterror(condition,msg,...) {if ((condition)) { char error[200] = "ERROR: %s::%u:\n\t";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);assert(false);}}
#define stmessage(condition,msg,...)  {if ((condition)) { char error[200] = "WARNING: %s::%u:\n";strcat(error,msg);strcat(error,"\n");fprintf(stderr,error,__FILE__,__LINE__ , ## __VA_ARGS__);}}

#else

#define stwarning(msg,...) {;}
#define sterror(condition,msg,...) {;}
#define stmessage(condition,msg,...)  {;}

#endif

#endif



#endif


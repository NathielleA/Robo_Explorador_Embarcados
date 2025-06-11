/*
 * File: rtwtypes.h
 *
 * MATLAB Coder version            : 5.2
 * C/C++ source code generated on  : 06-Jun-2025 09:14:57
 */

#ifndef RTWTYPES_H
#define RTWTYPES_H

/*=======================================================================*
 * Fixed width word size data types:                                     *
 *   int64_T                      - signed 64 bit integers               *
 *   uint64_T                     - unsigned 64 bit integers             *
 *=======================================================================*/

#if defined(__APPLE__)
#ifndef INT64_T
#define INT64_T long
#define FMT64 "l"
#if defined(__LP64__) && !defined(INT_TYPE_64_IS_LONG)
#define INT_TYPE_64_IS_LONG
#endif
#endif
#endif

#if defined(__APPLE__)
#ifndef UINT64_T
#define UINT64_T unsigned long
#define FMT64 "l"
#if defined(__LP64__) && !defined(INT_TYPE_64_IS_LONG)
#define INT_TYPE_64_IS_LONG
#endif
#endif
#endif

// typedefs genéricos equivalentes ao tmwtypes.h
#ifndef TMWTYPES_DEFINED
#define TMWTYPES_DEFINED

typedef unsigned char boolean_T;
typedef unsigned char uint8_T;
typedef signed char int8_T;
typedef unsigned short uint16_T;
typedef short int16_T;
typedef unsigned int uint32_T;
typedef int int32_T;
typedef float real32_T;
typedef double real_T;

#endif

#endif
/*
 * File trailer for rtwtypes.h
 *
 * [EOF]
 */

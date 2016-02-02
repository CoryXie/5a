#ifndef __UTILS_H
#define __UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#define	nelem(x)	(sizeof(x)/sizeof((x)[0]))

typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long ulong;
typedef unsigned int uint;
typedef signed char schar;
typedef long vlong;
   
#ifdef __cplusplus
}
#endif
#endif
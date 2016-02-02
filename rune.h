#ifndef __RUNE_H
#define __RUNE_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>
 
#ifdef UINT32_C
#define RUNE_C(x) UINT32_C(x)
#if __cplusplus >= 201103L
typedef char32_t Rune;
#else
typedef uint_least32_t Rune;
#endif
#else
#define RUNE_C(x) x##UL
typedef unsigned long Rune;
#endif

#define UTFmax 6 /* maximum bytes per rune */

#define Runeerror 0xFFFD          /* the "error" Rune or "Unicode replacement character" */
#define Runeself 0x80             /* rune and UTF sequences are the same (<) - characters below Runeself are represented as themselves in a single byte. */
#define Runesync 0x80             /* cannot represent part of a UTF sequence (<) */
#define Runemax  RUNE_C(0x10FFFF) /* maximum rune value - 21-bit rune */


typedef unsigned char uchar;
 
#ifdef __cplusplus
extern "C" {
#endif

int runetochar(char *, const Rune *);
int charntorune(Rune *, const char *, size_t);
int chartorune(Rune *, const char *);
int fullrune(const char *, size_t);
int runelen(const Rune);
size_t runenlen(const Rune *, size_t);
size_t utflen(const char *);
size_t utfnlen(const char *, size_t);
char *utfrune(const char *, Rune);
char *utfrrune(const char *, Rune);
char *utfutf(const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif /* __RUNE_H */
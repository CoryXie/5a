#ifndef __BIO_H
#define __BIO_H

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef FILE * Biobuf;

int       
Binit(Biobuf *bp, int fd, int mode);

int
Bputc(Biobuf *bp, int c);

int
Bflush(Biobuf *bp);

long 
Bwrite(Biobuf *bp, void *addr, long nbytes);

#ifdef __cplusplus
}
#endif

#endif
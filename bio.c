#include "bio.h"

const char * mode2str(int mode) {
    if (mode == O_RDONLY)
        return "r";
    else if (mode == O_WRONLY)
        return "w";
    else if (mode == O_RDWR)
        return "rw";
    else
        return "a+";
} 

int
Binit(Biobuf *bp, int f, int mode) {
FILE * file = fdopen(f, mode2str(mode));

if (bp)
    *bp = (Biobuf)file;

if (file == NULL)
    return -1;
else
    return 0;
}

int
Bputc(Biobuf *bp, int c) {
return fputc(c, (FILE *)*bp);   
}

int
Bflush(Biobuf *bp) {
return fflush((FILE *)*bp);    
}

long 
Bwrite(Biobuf *bp, void *addr, long nbytes) {
return  fwrite(addr, nbytes, 1, (FILE *)*bp);       
}


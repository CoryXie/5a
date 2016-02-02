## Implementing `Binit()`, `Bputc()`, `Bflush()` and `Bwrite()`

```c

    Biobuf	obuf;
    
    const char * mode2str(int mode) {
        if (mode == O_RDONLY)
            return "r";
        else if (mode == O_WRONLY)
            reutrn "w";
        else if (mode == O_RDWR)
            reutrn "rw";
        else
            reutrn "a+";
    } 

    int
    Binit(Biobuf *bp, int f, int mode) {
    FILE * file = fdopen(f, mode2str(mode));

    if (bp)
        *bp = file;

    if (file == NULL)
        return -1;
    else
        reutrn 0;
    }

    int
    Bputc(Biobuf *bp, int c) {
    return fputc(c, bp);   
    }
    
    long 
    Bwrite(Biobuf *bp, void *addr, long nbytes) {
    return  fwrite(addr, nbytes, 1, (FILE *)*bp);       
    }
    
    int
    Bflush(Biobuf *bp) {
    return fflush(bp);    
    }

```

## Replace `getc()` with `lexgetc()`

```console

    build/debug/a.h:135:5: error: conflicting types for '_IO_getc'
     int getc(void);
         ^
    In file included from /usr/include/stdio.h:74:0,
                     from build/debug/a.h:1,
                     from build/debug/a.y:2:
    /usr/include/libio.h:434:12: note: previous declaration of '_IO_getc' was here
     extern int _IO_getc (_IO_FILE *__fp);
                ^
    scons: *** [build/debug/a.o y.tab.h] Error 1

```

This can be solved by replacing `getc()` with `lexgetc()`.

```console

    $find . -type f -exec sed -i 's/getc/lexgetc/g' {} \;

```

## Debugging

```console
    $ gdb --args build/debug/5a l.s
```
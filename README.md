# Plan 9 ARM Assembler (5A) Linux Port #

This is a standalone Linux port for the Plan 9 ARM Assembler (5A).

## Changes

See `docs/Changes.md` for details.

## Building

```console

	$scons
	scons: Reading SConscript files ...
	scons: done reading SConscript files.
	scons: Building targets ...
	scons: building associated VariantDir targets: build/release build/debug
	bison --defines=y.tab.h -o build/debug/a.c a.y
	gcc -o build/debug/a.o -c -ggdb -DVERSION=1 -DDEBUG -Ibuild/debug -I. build/debug/a.c
	gcc -o build/debug/lex.o -c -ggdb -DVERSION=1 -DDEBUG -Ibuild/debug -I. lex.c
	gcc -o build/debug/rune.o -c -ggdb -DVERSION=1 -DDEBUG -Ibuild/debug -I. rune.c
	gcc -o build/debug/5a build/debug/a.o build/debug/lex.o build/debug/rune.o
	bison --defines=y.tab.h -o build/release/a.c a.y
	gcc -o build/release/a.o -c -O2 -DVERSION=1 -DRELEASE -Ibuild/release -I. build/release/a.c
	gcc -o build/release/lex.o -c -O2 -DVERSION=1 -DRELEASE -Ibuild/release -I. lex.c
	gcc -o build/release/rune.o -c -O2 -DVERSION=1 -DRELEASE -Ibuild/release -I. rune.c
	gcc -o build/release/5a build/release/a.o build/release/lex.o build/release/rune.o
	scons: done building targets.

```

## Testing

```console

	$./build/debug/5a test/l.s

```


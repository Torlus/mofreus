mofreus
---
Basic BMP compression, applying RLE to MOst FREquently USed bytes only.

Update: algorithm has been changed to use Span Use Size Improvement, i.e check if using RLE on a given byte value actually improves the compression.

# About

Simple compression and uncompression routines, with no library dependency.

Performs RLE on a subset of characters only.

The C code has been written in a way that it can easily (read: automatically) be converted into Java code.

# Building

## Sample test program

```
gcc -c mofreus.c
gcc -c mofreus_test.c
gcc mofreus.o mofreus_test.o -o ./bin/mofreus_test
```

## Java source code

```
gcc -DJAVA -E -P mofreus.c > Mofreus.java
```

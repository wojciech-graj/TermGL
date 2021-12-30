# TermGL

A terminal-based graphics library for both 2D and 3D graphics.\
Works in all terminals supporting ANSI escape codes.\
C11 compliant.

A single-header version created by [assyrianic](https://github.com/assyrianic) can be found in [their repo](https://github.com/assyrianic/TermGL). It will not have ongoing support.

## Table of Contents

[Gallery](https://github.com/wojciech-graj/TermGL/blob/master/README.md#Gallery)\
[Build](https://github.com/wojciech-graj/TermGL/blob/master/README.md#Build)\
[Documentation](https://github.com/wojciech-graj/TermGL/blob/master/README.md#Documentation)

## Gallery

![LOGO](test/logo.gif)

![CANYON](test/canyon.gif)

![TEAPOT](test/teapot.gif)

## Build

TermGL has no external dependencies and only relies on the C standard library.\
To use TermGL, include [termgl.h](src/termgl.h) and compile [termgl.c](src/termgl.c). For 3D functionality, TERMGL3D has to be defined by passing -DTERMGL3D to the compiler.\
Certain settings can be changed in [termgl.h](src/termgl.h), e.g. memory allocation functions, clear screen command, compiler-specific commands.\
To compile a test program, run the [test/Makefile](test/Makefile).
```
make test
```

## Documentation

A sample program exists here: [test/termgl_test.c](test/termgl_test.c), and utilizes all major features of the TermGL library.\
The header file [termgl.h](src/termgl.h) contains brief documentation for all functions and structs.\
Compiler-specific (GCC) macros are used for loop unrolling in the ```tgl_mulmat``` and ```clip_triangle_plane``` functions

# TermGL

A terminal-based graphics library for both 2D and 3D graphics.\
Works in all terminals supporting ANSI escape codes.\
C11 compliant, only reliant on the C standard library.

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

### C

#### Method 1: Regular source file

1. Add the following flags to your compiler ```-I/path/to/TermGL/lib -lm```
2. Add termgl.c as a source file to be compiled
3. (Optional) Enable 3D functionality with the compiler flag ```-DTERMGL3D```

#### Method 2: Shared library

1. Run the makefile ```make shared``` (NOTE: 3D functionality is enabled by default. Remove ```-DTERMGL3D``` from Makefile CFLAGS to disable.)
2. Add the following flags to your compiler ```-I/path/to/TermGL/lib -L/path/to/TermGL/lib -ltermgl -lm```
3. (Optional) Enable 3D functionality with the compiler flag ```-DTERMGL3D```

### C++

The above Method 2 for C can be used to use TermGL in C++

### Demo

To compile a demo program, run the [Makefile](Makefile). ```make demo```

## Documentation

Certain settings can be changed in [termgl.h](src/termgl.h), e.g. memory allocation functions, clear screen command, compiler-specific commands.\
A sample program exists here: [test/termgl_test.c](test/termgl_test.c), and utilizes all major features of the TermGL library.\
The header file [termgl.h](src/termgl.h) contains brief documentation for all functions and structs.\
Compiler-specific (GCC) macros are used for loop unrolling in the ```tgl_mulmat``` and ```clip_triangle_plane``` functions

# TermGL

A terminal-based graphics library for both 2D and 3D graphics.\
Written in C, created for terminals supporting ANSI escape codes.

A single-header version created by [assyrianic](https://github.com/assyrianic) can be found in [their repo](https://github.com/assyrianic/TermGL) or [this branch](https://github.com/wojciech-graj/TermGL/tree/single-header)

## Table of Contents
[Gallery](https://github.com/wojciech-graj/TermGL/blob/master/README.md#Gallery)\
[Build](https://github.com/wojciech-graj/TermGL/blob/master/README.md#Build)\
[Documentation](https://github.com/wojciech-graj/TermGL/blob/master/README.md#Documentation)

## Gallery

![LOGO](test/logo.gif)

![CANYON](test/canyon.gif)

![TEAPOT](test/teapot.gif)

## Build

Only C standard libraries are used, allowing for easy compilation.\
To compile a test program, run the [test/Makefile](test/Makefile).
```
make test
```

## Documentation

A sample program exists here: [test/termgl_test.c](test/termgl_test.c), and utilizes all features of the TermGL library.\
Every header file in [src/](src/) also contains documentation for every function.\
Certain settings can be changed in the termgl.h file, e.g. memory allocation functions, clear screen command, compiler-specific commands.

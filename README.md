# TermGL

A terminal-based graphics library for 2D and 3D graphics.

Features:
- Windows & *NIX support
- C99 compliant without external dependencies
- Custom vertex and pixel shaders
- Affine texture mapping
- 24 bit RGB
- Indexed color mode: 16 Background colors, 16 foreground colors, bold and underline
- Non-blocking input from terminal

### Gallery

![LOGO](demo/logo.gif)

![CUBE](demo/textures.gif)

![CANYON](demo/canyon.gif)

### Build

You can compile `termgl.c` as you would any other C source file. You can also compile it as a shared library `libtermgl.so` by calling `make shared`. To install the shared library, `sudo make install`.

To enable 3D functionality, use the ```-DTERMGL3D``` compiler flag.\
To enable utility functions, use the ```-DTERMGLUTIL``` compiler flag.\
To disable helper functions for vector math and shaders, use the ```-DTERMGL_MINIMAL``` compiler flag.

To use TermGL in C++, compile it as a shared library and link against the `libtermgl.so` file. The `termgl.h` header can be included from C++ files.

To compile a demo program, run  `make demo`, creating the `termgl_demo` binary.

To cross-compile, set the `CC` variable to the appropriate compiler.

### Documentation

Certain settings can be changed at the top of `termgl.c` prior to compilation, e.g. memory allocation functions, clear screen command, compiler-specific commands.\
The header file `termgl.h` contains brief documentation for all functions and structs.\
Compiler-specific functionality is used, therefore it is recommened to always compile using GCC.\
The TermGLUtil extension contains functions for reading keyboard input, but requires either Windows or *NIX headers.

### Demo

A demo program can be found at `demo/termgl_demo.c`.\
Available demos and TermGL features used:
1. Utah Teapot\
Renders a rotating 3D Utah Teapot.
	- Backface culling
	- Z buffering
	- Double-width characters
	- 3D rendering
	- Custom shaders
2. Color Palette\
Renders a palette of various text colors and styles.
	- Colors & Modifiers
3. Mandelbrot\
Renders an infinitely zooming-in Mandelbrot set.
	- Point rendering
4. Realtime Keyboard\
Displays keyboard input in realtime.
	- Text rendering
	- Realtime keyboard input
5. Textured Cube\
Renders a texture-mapped cube.
	- Backface culling
	- Z buffering
	- Double-width characters
	- 3D rendering
	- Shaders
	- Texture mapping
6. RGB\
Renders overlapping red, green, and blue circles.
	- 24 bit RGB
	- Text rendering

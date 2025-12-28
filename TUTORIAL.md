# TermGL Tutorial

This tutorial provides a quick overview of TermGL's functionality. Please reference the [demo](./termgl_demo.c), and read the function documentation in [`termgl.h`](./termgl.h) for more details.

## Getting Started

Most operations require a rendering context (`TGL *`). We can construct a context by specifying the width and height in characters - in this case we have 40 columns and 24 rows. The `tgl_puts` function is used to print a string at given coordinates with a given color. Finally, we have to call `tgl_flush` to print the current frame. `tgl_clear` is used to clear the frame buffer, so we don't print the last frame next time we call `tgl_flush`. Finally, once you're done using the context, free it with `tgl_delete`.

```c
TGL *const tgl = tgl_init(40, 24);
tgl_puts(tgl, 0, 0, "Hello World!", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
tgl_flush(tgl);
tgl_clear(tgl, TGL_FRAME_BUFFER);
tgl_delete(tgl);
```

## Colors

*See also:* [demo_color](./termgl_demo.c), [demo_rgb](./termgl_demo.c)

TermGL supports both indexed colors and 24 bit RGB. Drawing functions require a `TGLPixFmt`, which specifies the colors of the foreground character and the background behind it.

```c
TGLFmt idx1 = TGL_IDX(TGL_WHITE);
TGLFmt idx2 = TGL_IDX(TGL_RED | TGL_HIGH_INTENSITY, TGL_UNDERLINE);
TGLFmt rgb1 = TGL_RGB(1, 2, 3);
TGLFmt rgb2 = TGL_RGB(1, 2, 3, TGL_BOLD | TGL_UNDERLINE);

TGLPixFmt color1 = TGL_PIXFMT(idx1);  /* only foreground */
TGLPixFmt color2 = TGL_PIXFMT(rgb2, idx2);  /* foreground and background */
```

## Settings

TermGL has multiple settings that can be changed in the rendering context using the `tgl_enable` and `tgl_disable` functions.

```c
TGL *const tgl = tgl_init(40, 24);
tgl_enable(tgl, TGL_OUTPUT_BUFFER | TGL_CULL_FACE);
tgl_delete(tgl);
```

The following settings are available:

- `TGL_OUTPUT_BUFFER`: Output buffer allowing for just one print to flush. Much faster on most terminals, but requires a few hundred kilobytes of memory.
- `TGL_Z_BUFFER`: Depth buffer.
- `TGL_DOUBLE_WIDTH`: Display characters at double their standard widths (Limited support from terminal emulators. Should work on Windows Terminal, XTerm, and Konsole).
- `TGL_DOUBLE_CHARS`: Square pixels by printing 2 characters per pixel.
- `TGL_PROGRESSIVE`: Over-write previous frame. Eliminates strobing but requires call to `tgl_clear_screen` before drawing smaller image and after resizing terminal if terminal size was smaller than frame size.
- `TGL_CULL_FACE`: (3D ONLY) Cull specified triangle faces

## Text Rendering

Text rendering can be performed through either the `tgl_putchar` or `tgl_puts` functions, to print characters and strings respectively.

```c
TGL *const tgl = tgl_init(40, 24);
tgl_putchar(tgl, 0, 1, 'a', TGL_PIXFMT(TGL_IDX(TGL_RED)));
tgl_putchar(tgl, 0, 2, 'b', TGL_PIXFMT(TGL_IDX(TGL_BLUE)));
tgl_putchar(tgl, 0, 3, 'c', TGL_PIXFMT(TGL_IDX(TGL_GREEN)));
tgl_puts(tgl, 0, 0, "Hello World!", TGL_PIXFMT(TGL_IDX(TGL_WHITE)));
tgl_flush(tgl);
tgl_delete(tgl);
```

## 2D Rendering

*See also:* [demo_mandelbrot](./termgl_demo.c)

2D rendering can be performed through the `tgl_point`, `tgl_line`, `tgl_triangle`, and `tgl_triangle_fill` functions.

Each vertex has `u` and `v` values between 0 and 255 that, for lines and triangles, get interpolated between points. The color of each pixel being drawn is determined by a `TGLPixelShader`, which receives these `u` and `v` values, and calculates the color and character that will be used for that pixel.

You can implement your own `TGLPixelShader`s, but TermGL also provides a `TGLPixelShaderSimple` that uses a constant color and uses characters from a `TGLGradient`, and a `TGLPixelShaderTexture` that allows for 2D textures to be applied. TermGL provides the `tgl_gradient_full` and `tgl_gradient_min` gradients, and allows for user-defined `TGLGradient`s for the `TGLPixelShaderSimple`.

```c
TGLVert v0 = (TGLVert){
	.x = 19,
	.y = 1,
	.z = 0.0,
	.u = 0,
	.v = 0,
};
TGLVert v1 = (TGLVert){
	.x = 1,
	.y = 11,
	.z = 0.0,
	.u = 127,
	.v = 0,
};
TGLVert v2 = (TGLVert){
	.x = 39,
	.y = 11,
	.z = 0.0,
	.u = 0,
	.v = 127,
};

TGLPixelShaderSimple shader_trig = (TGLPixelShaderSimple){
	.color = TGL_PIXFMT(TGL_IDX(TGL_WHITE)),
	.grad = &tgl_gradient_full,
};
TGLPixelShaderSimple shader_line = (TGLPixelShaderSimple){
	.color = TGL_PIXFMT(TGL_IDX(TGL_RED)),
	.grad = &tgl_gradient_min,
};

TGL *const tgl = tgl_init(40, 24);
tgl_triangle_fill(tgl, v0, v1, v2, &tgl_pixel_shader_simple, &shader_trig);
tgl_line(tgl, v0, v1, &tgl_pixel_shader_simple, &shader_line);
tgl_line(tgl, v1, v2, &tgl_pixel_shader_simple, &shader_line);
shader_line.color = TGL_PIXFMT(TGL_IDX(TGL_BLUE));
tgl_point(tgl, v2, &tgl_pixel_shader_simple, &shader_line);
tgl_flush(tgl);
tgl_delete(tgl);
```

## 3D Rendering

*See also:* [demo_teapot](./termgl_demo.c), [demo_texture](./termgl_demo.c)

3D rendering can be performed through the `tgl_triangle_3d` function.

To only draw triangles from one side, you should enable `TGL_CULL_FACE`, and call `tgl_cull_face` to specify which faces you wish to cull.

## Mouse, Keyboard, and Utilities

*See also:* [demo_keyboard](./termgl_demo.c), [demo_mouse](./termgl_demo.c)

On Windows and Linux, TermGL provides functionality for reading mouse and keyboard events, and controlling other aspects of the terminal.

`tglutil_get_console_size` and `tglutil_set_console_size` are used to manage the size of the console, and `tglutil_set_window_title` will attempt to set the window's title. If you don't want the user's input to be visible, or when using mouse tracking, you can disable echoing of user input with `tglutil_set_echo_input`. Mouse tracking can be enabled and disabled with `tglutil_set_mouse_tracking_enabled`.

Keyboard and mouse input can be read in real time using the `tglutil_read` function.

Don't forget to re-enable echoing and re-disable mouse tracking when exiting your program.

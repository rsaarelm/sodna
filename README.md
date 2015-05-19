Sodna is a lightweight virtual console library for text mode games.

Current implementation uses SDL 2.0 from http://libsdl.org/ as a
backend.

To build the demo application, use premake:

    $ premake4 gmake
    $ make

Linux users will want to have premake4 and SDL2 installed using
their system package manager. Windows versions are included with
Sodna.

Files
-----

* `include/sodna.h`: The base Sodna API header. Also what passes for
  API documentation at the moment. Implementations of Sodna, like
  the initial SDL2 one, are written against this.

* `include/sodna_util.h`: Header for non-essential utility methods
  that are implemented on top of the base API.

* `src/sodna_sdl2.c`: SDL2 implementation of the base Sodna API.

* `src/sodna_default_font.inc`: Embedded binary for the default
  Sodna font. Needed by `sodna_sdl2.c`.

* `src/sodna_util.c`: Implementation for the non-essential Sodna
  utilities.

* `src/stb_image.h`, `src/stb_image_write.h`: STB image library by
  Sean Barrett, used by `sodna_util.c`

* `src_demo/demo.c`: Messy example program.

Notes
-----

* Use `tools/bake_font.sh` to create your own baked-in font include
  file from a 16 columns by 16 rows left-to-right bitmap font sheet.
  Eg.

      $ ./tools/bake_font.sh my_font.png > my_font.inc

  The tool requires that you have ImageMagick installed and in your
  command line path.

* See `codepage_437.txt` for making your own font sheet image.

Bugs
----

* `sodna_util.c` can't read font sheet pngs processed with optipng due
  to limits in `stb_image`.
* Due to [bug](https://bugzilla.libsdl.org/show_bug.cgi?id=2736) in
  SDL2, pressing caps lock when Sodna window is out of focus can
  make the caps lock state reported by Sodna inconsistent with the
  actual keyboard state.

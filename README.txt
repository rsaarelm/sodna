Sodna is a lightweight virtual console library for text mode games.

Current implementation uses SDL 2.0 from http://libsdl.org/ as a
backend.

Notes
-----

* Use tools/bake_font.sh to create your own baked-in font include
  file from a 16x16 left-to-right bitmap font sheet. See Makefile
  for an usage example.
* See codepage_437.txt for making your own font sheet image.

Bugs
----

* sodna_util can't read font sheet pngs processed with optipng due
  to limits in stb_image.
* The centered, pixel-perfect canvas scaling does not work right on
  a tiling window manager like i3wm after calling sodna_resize.

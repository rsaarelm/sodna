CC = gcc
CFLAGS = -Iinclude -Wall -Werror -O3 -lm
LDFLAGS =
AR = ar

all: libsodna_sdl2.so libsodna_sdl2.a demo

sodna_sdl2.o: include/sodna.h src/sodna_sdl2.c src/font8.inc
	$(CC) $(CFLAGS) -fPIC $$(sdl2-config --cflags) -c src/sodna_sdl2.c -o $@

libsodna_sdl2.so: sodna_sdl2.o
	$(CC) $(LDFLAGS) $< -shared -o $@ 

libsodna_sdl2.a: sodna_sdl2.o
	$(AR) $(LDFLAGS) rcs $@ $< 

demo: libsodna_sdl2.so src_demo/demo.c src/sodna_util.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Isodna_util -L. -lsodna_sdl2 -Wl,-rpath=. -o $@ `sdl2-config --libs` src_demo/demo.c src/sodna_util.c

clean:
	rm -f *.o *.so *.a demo

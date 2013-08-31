#include "sodna.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

const float pi = 3.14159;

sodna_Cell plasma(float t, float x, float y) {
    float v = 0.f;
    v += sin(x + t);
    v += sin((y + t) / 2.f);;
    v += sin((x + y + t) / 2.f);;

    return (sodna_Cell){ '5' + 5.f * sin(pi * v), 15, 15, 15, 15, 15 * sin(pi * v), 15 * cos(pi * v)};
}

void chaos() {
    // Test non-blocking animation.
    float t = 0.f;
    for (;;) {
        int x, y;
        for (y = 0; y < sodna_height(); y++) {
            for (x = 0; x < sodna_width(); x++) {
                sodna_cells()[x + sodna_width() * y] =
                    plasma(t, (float)x / sodna_width(), (float)y / sodna_height());
            }
        }
        t += 0.01f;
        sodna_flush();
        if (sodna_poll_event())
            break;
    }

    memset(sodna_cells(), 0, sodna_width() * sodna_height() * sizeof(sodna_Cell));
}

const char* terrain[] = {
    "####  ####",
    "#  ####  #",
    "#        #",
    "##      ##",
    " #      # ",
    " #      # ",
    "##      ##",
    "#        #",
    "#  ####  #",
    "####  ####"};

void draw_map(int player_x, int player_y) {
    const sodna_Cell wall =   { '#', 9,  9,  9, 0, 2, 1 };
    const sodna_Cell floor =  { ' ', 9,  9,  9, 0, 2, 1 };
    const sodna_Cell player = { '@', 8, 13, 15, 0, 2, 1 };
    int y;
    sodna_Cell* cells = sodna_cells();

    for (y = 0; y < sizeof(terrain)/sizeof(char*); y++) {
        int x = 0;
        do {
            int offset = y * sodna_width() + x;
            if (x == player_x && y == player_y) {
                cells[offset] = player;
            } else {
                cells[offset] = terrain[y][x] == '#' ? wall : floor;
            }
        } while (terrain[y][++x]);
    }
}

int can_enter(int x, int y) {
    return x >= 0 && y >= 0 && y < sizeof(terrain)/sizeof(char*) && x < strlen(terrain[y]) && terrain[y][x] != '#';
}

void simpleRl() {
    int x = 2, y = 2;
    for (;;) {
        int dx = 0, dy = 0;
        draw_map(x, y);
        sodna_flush();
        switch (sodna_wait_event()) {
            case SODNA_CLOSE_WINDOW:
            case SODNA_ESC:
                return;
            case SODNA_UP:
            case 'w':
                dy = -1;
                break;
            case SODNA_RIGHT:
            case 'd':
                dx = 1;
                break;
            case SODNA_DOWN:
            case 's':
                dy = 1;
                break;
            case SODNA_LEFT:
            case 'a':
                dx = -1;
                break;
            default:
                break;
        }

        if (can_enter(x + dx, y + dy)) {
            x += dx;
            y += dy;
        }
    }
}

int main(int argc, char* argv[]) {
    sodna_init();
    chaos();
    simpleRl();
    sodna_exit();
    return 0;
}

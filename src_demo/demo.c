#include "sodna.h"
#include <string.h>
#include <stdlib.h>

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

int main(int argc, char* argv[]) {
    int x = 2, y = 2;
    sodna_init();

    // Test non-blocking animation.
    for (;;) {
        int i = sodna_width() * sodna_height() + 1;
        while (i) {
            uint32_t* cells = (uint32_t*)sodna_cells();
            cells[--i] = rand();
        }
        sodna_flush();
        if (sodna_poll_event())
            break;
    }

    memset(sodna_cells(), 0, sodna_width() * sodna_height() * sizeof(sodna_Cell));

    for (;;) {
        int dx = 0, dy = 0;
        draw_map(x, y);
        sodna_flush();
        switch (sodna_wait_event()) {
            case SODNA_CLOSE_WINDOW:
            case SODNA_ESC:
                goto exit;
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
exit:
    return 0;
}

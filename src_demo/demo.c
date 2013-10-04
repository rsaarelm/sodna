#include "sodna.h"
#include <string.h>
#include <stdlib.h>

static uint8_t flame_buffer[27][82];

void update_flame() {
    int x, y;
    for (x = 0; x < 82; x++) {
        flame_buffer[26][x] = rand() % 3 ? 0 : 255;
    }
    for (y = 0; y < 26; y++) {
        for (x = 1; x < 81; x++) {
            int sum = flame_buffer[y][x-1] + flame_buffer[y][x+1];
            sum += flame_buffer[y+1][x-1] + flame_buffer[y+1][x] + flame_buffer[y+1][x+1];
            sum /= 5;
            flame_buffer[y][x] = sum;
        }
    }
}

void chaos() {
    // Test non-blocking animation.
    for (;;) {
        int x, y;
        update_flame();
        for (y = 0; y < sodna_height(); y++) {
            for (x = 0; x < sodna_width(); x++) {
                int i = flame_buffer[y][x+1] / 8;
                int r = i > 15 ? 15 : i;
                int g = i > 15 ? i - 16 : 0;
                int b = 0;
                sodna_cells()[x + sodna_width() * y] =
                    (sodna_Cell){ ' ', 0, 0, 0, b, g, r };
            }
        }
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
    const sodna_Cell wall =   { '#', 9,  9,  9, 0, 0, 0 };
    const sodna_Cell floor =  { ' ', 9,  9,  9, 0, 0, 0 };
    const sodna_Cell player = { '@', 15, 13, 8, 0, 0, 0 };
    int y;
    sodna_Cell* cells = sodna_cells();

    for (y = 0; y < sizeof(terrain)/sizeof(char*); y++) {
        int x = 0;
        do {
            int offset = y * sodna_width() + x * 2;
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
    sodna_init(8, 16, 80, 25, "Sodna demo");
    chaos();
    simpleRl();
    sodna_exit();
    return 0;
}

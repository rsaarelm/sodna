#include "sodna.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "sodna_util.h"

static int g_is_fullscreen = 0;

static sodna_Cell cell(char c, int fore, int back) {
    sodna_Cell ret;
    ret.symbol = c;
    ret.fore.b = (fore & 0x00f) << 4;
    ret.fore.g = (fore & 0x0f0);
    ret.fore.r = (fore & 0xf00) >> 4;
    ret.back.b = (back & 0x00f) << 4;
    ret.back.g = (back & 0x0f0);
    ret.back.r = (back & 0xf00) >> 4;

    return ret;
}

void wait_enter() {
    sodna_Event e;
    for (;;) {
        e = sodna_wait_event(1000);
        if (e.type == SODNA_EVENT_KEY_DOWN && e.key.layout == SODNA_KEY_ENTER)
            return;
        if (e.type == SODNA_EVENT_CHARACTER)
            printf("%c (\\u%04x)\n", e.ch.code, e.ch.code);
        sodna_flush();
    }
}

void test_screen() {
    int x, y;
    sodna_Cell* cells = sodna_cells();
    for (y = 0; y < 16; y++) {
        for (x = 0; x < 16; x++) {
            int offset = y * sodna_width() + 60 + x;
            cells[offset] = cell(x + 16*y, 0x280, 0x000);
        }
    }
    sodna_flush();
    wait_enter();
}

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
    int mx = -1, my = -1;
    /* Test non-blocking animation. */
    for (;;) {
        int x, y;
        sodna_Event e;
        update_flame();
        for (y = 0; y < sodna_height(); y++) {
            for (x = 0; x < sodna_width(); x++) {
                int i = flame_buffer[y][x+1] / 8;
                int r = i > 15 ? 15 : i;
                int g = i > 15 ? i - 16 : 0;
                int b = 0;
                sodna_cells()[x + sodna_width() * y] =
                    cell(' ', 0x000, (r << 8) + (g << 4) + b);
            }
        }
        if (mx >= 0 && my >= 0 && mx < sodna_width() && my < sodna_height())
            sodna_cells()[mx + sodna_width() * my] = cell('X', 0x000, 0x0f0);
        do {
            e = sodna_poll_event();
            switch (e.type) {
                /* Track mouse */
                case SODNA_EVENT_MOUSE_MOVED:
                    mx = e.mouse.x;
                    my = e.mouse.y;
                    break;
                /* Halt animation if focus is lost. */
                case SODNA_EVENT_FOCUS_LOST:
                    do {
                        e = sodna_wait_event(0);
                    } while (e.type != SODNA_EVENT_FOCUS_GAINED &&
                            e.type != SODNA_EVENT_CLOSE_WINDOW);
                    break;
                case SODNA_EVENT_MOUSE_DOWN:
                    goto exit;
            }
            if (e.type == SODNA_EVENT_KEY_DOWN && e.key.layout == SODNA_KEY_ENTER)
                goto exit;
        } while (e.type != SODNA_EVENT_NONE);
        sodna_flush();
    }

exit:
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
    const sodna_Cell wall = cell('#', 0x999, 0x000);
    const sodna_Cell floor =  cell(' ', 0x999, 0x000);
    const sodna_Cell player = cell('@', 0xFD8, 0x000);
    int x, y;
    sodna_Cell* cells = sodna_cells();

    for (y = 0; y < sizeof(terrain)/sizeof(char*); y++) {
        x = 0;
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
        sodna_Event e;
        draw_map(x, y);
        do {
            e = sodna_poll_event();
            if (e.type == SODNA_EVENT_CLOSE_WINDOW)
                return;
        } while (e.type != SODNA_EVENT_NONE && e.type != SODNA_EVENT_KEY_DOWN);
        sodna_flush();

        switch (e.key.hardware) {
            case SODNA_KEY_ESCAPE:
                return;
            case SODNA_KEY_UP:
            case SODNA_KEY_W:
                dy = -1;
                break;
            case SODNA_KEY_RIGHT:
            case SODNA_KEY_D:
                dx = 1;
                break;
            case SODNA_KEY_DOWN:
            case SODNA_KEY_S:
                dy = 1;
                break;
            case SODNA_KEY_LEFT:
            case SODNA_KEY_A:
                dx = -1;
                break;
            case SODNA_KEY_F12:
                sodna_save_screenshot_png("sodna_demo.png");
            default:
                break;
        }

        // Alt-enter to toggle fullscreen.
        if (e.key.alt && e.key.layout == SODNA_KEY_ENTER) {
            sodna_set_fullscreen(!g_is_fullscreen);
            g_is_fullscreen = !g_is_fullscreen;
        }

        if (can_enter(x + dx, y + dy)) {
            x += dx;
            y += dy;
        }
    }
}

int main(int argc, char* argv[]) {
    // Load a custom font.
    sodna_Font* font = NULL;
    sodna_load_font("font/8x14.png", &font);
    sodna_init(80, 25, "Sodna demo", font);
    free(font);

    test_screen();
    chaos();
    simpleRl();
    sodna_exit();
    return 0;
}

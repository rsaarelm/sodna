#include "sodna.h"
#include <string.h>
#include <stdlib.h>

#include "stb_image.h"

static sodna_Cell cell(char c, int fore, int back) {
    sodna_Cell ret;
    ret.symbol = c;
    ret.fore_b = fore;
    ret.fore_g = fore >> 4;
    ret.fore_r = fore >> 8;
    ret.back_b = back;
    ret.back_g = back >> 4;
    ret.back_r = back >> 8;

    return ret;
}

int is_keydown(int e) {
    if (e > 0) {
        return SODNA_SCANCODE(e) > 0;
    }
    return 0;
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
    while (!is_keydown(sodna_wait_event(1000)));
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
        int e = 0;
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
            switch (SODNA_EVENT(e)) {
                /* Track mouse */
                case SODNA_MOUSE_MOVED:
                    mx = SODNA_EVENT_X(e), my = SODNA_EVENT_Y(e);
                    break;
                /* Halt animation if focus is lost. */
                case SODNA_FOCUS_LOST:
                    do {
                        e = sodna_wait_event(1000);
                    } while (e != SODNA_FOCUS_GAINED && e != SODNA_CLOSE_WINDOW);
                    break;
                case SODNA_MOUSE_DOWN:
                    goto exit;
            }
            if (is_keydown(e))
                goto exit;
        } while (e);
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
        int e = 0;
        draw_map(x, y);
        sodna_flush();
        do { e = sodna_wait_event(1000); } while (e < SODNA_CLOSE_WINDOW);
        if (e == SODNA_CLOSE_WINDOW)
            return;
        if (e <= 0)
            continue;

        switch (SODNA_SCANCODE(e)) {
            case SODNA_SCANCODE_ESCAPE:
                return;
            case SODNA_SCANCODE_UP:
            case SODNA_SCANCODE_W:
                dy = -1;
                break;
            case SODNA_SCANCODE_RIGHT:
            case SODNA_SCANCODE_D:
                dx = 1;
                break;
            case SODNA_SCANCODE_DOWN:
            case SODNA_SCANCODE_S:
                dy = 1;
                break;
            case SODNA_SCANCODE_LEFT:
            case SODNA_SCANCODE_A:
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
    int w, h, n;
    uint8_t* data = stbi_load("8x16.png", &w, &h, &n, 1);
    sodna_init(8, 16, 80, 25, "Sodna demo");
    /* Test screen with default font. */
    test_screen();
    if (data) {
        sodna_load_font_data(data, w, h, 0);
    } else {
        fprintf(stderr, "Couldn't load font bitmap: %s\n",
                stbi_failure_reason());
    }
    stbi_image_free(data);
    /* Test screen with user font. */
    test_screen();
    chaos();
    simpleRl();
    sodna_exit();
    return 0;
}

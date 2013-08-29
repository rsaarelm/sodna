#include "sodna.h"
#include <SDL.h>

static SDL_Window* g_win = NULL;
static SDL_Renderer* g_rend = NULL;
static SDL_Texture* g_texture = NULL;
static Uint32* g_pixels = NULL;
static sodna_Cell* g_cells = NULL;

const int width = 640;
const int height = 400;

// TODO support different fonts and font sizes.
const int font_w = 8;
const int font_h = 16;
const uint8_t font[] = {
#include "font16.inc"
};

static inline int font_pixel(uint8_t symbol, int x, int y) {
    // XXX: Only handles font_w == 8 for now.
    return font[symbol * font_h + y] & (1 << (7 - x));
}

static inline void cell_to_argb(Uint32* out_fore, Uint32* out_back, sodna_Cell cell) {
    *out_fore = 0xff000000 | cell.fore_r << 20 | cell.fore_g << 12 | cell.fore_b << 4;
    *out_back = 0xff000000 | cell.back_r << 20 | cell.back_g << 12 | cell.back_b << 4;
}

int sodna_init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return 1;

    g_win = SDL_CreateWindow(
            "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN);
    if (!g_win)
        return 1;

    g_rend = SDL_CreateRenderer(g_win, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    g_texture = SDL_CreateTexture(
            g_rend, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            width, height);

    g_pixels = (Uint32*)malloc(width * height * 4);
    g_cells = (sodna_Cell*)malloc(sodna_width() * sodna_height() * sizeof(sodna_Cell));
    memset(g_cells, 0, sodna_width() * sodna_height() * sizeof(sodna_Cell));

    return 0;
}

void sodna_exit() {
    SDL_DestroyRenderer(g_rend); g_rend = NULL;
    SDL_DestroyWindow(g_win); g_win = NULL;
    SDL_DestroyTexture(g_texture); g_texture = NULL;
    free(g_pixels); g_pixels = NULL;
    free(g_cells); g_cells = NULL;
    SDL_Quit();
}

sodna_Cell* sodna_cells() {
    return g_cells;
}

void draw_cell(int x, int y, Uint32 fore_col, Uint32 back_col, uint8_t symbol) {
    int u, v;
    for (v = 0; v < font_h; v++) {
        size_t offset = x + (y + v) * width;
        for (u = 0; u < font_w; u++) {
            g_pixels[offset++] = font_pixel(symbol, u, v) ? fore_col : back_col;
        }
    }
}

void sodna_flush() {
    int x, y;
    sodna_Cell* cells = sodna_cells();
    // XXX: Always repaints all cells, even if there was no change from
    // previous frame. Could use a twin cell buffer and check for change
    // from previous frame to see if we can skip draw_cell.
    for (y = 0; y < sodna_height(); y++)
        for (x = 0; x < sodna_width(); x++) {
            Uint32 fore, back;
            sodna_Cell cell = cells[x + sodna_width() * y];
            cell_to_argb(&fore, &back, cell);
            draw_cell(x * font_w, y * font_h, fore, back, cell.symbol);
        }
    SDL_RenderClear(g_rend);
    SDL_UpdateTexture(g_texture, NULL, g_pixels, width * sizeof(Uint32));
    SDL_RenderCopy(g_rend, g_texture, NULL, NULL);
    SDL_RenderPresent(g_rend);
}

int sodna_width() {
    return width / font_w;
}

int sodna_height() {
    return height / font_h;
}

int sodna_wait_event() {
    SDL_Event event;
    do {
        SDL_WaitEvent(&event);
    } while (event.type != SDL_KEYDOWN);

    // TODO
    return 0;
}

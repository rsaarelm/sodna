#include "sodna.h"
#include <SDL.h>

static SDL_Window* g_win = NULL;
static SDL_Renderer* g_rend = NULL;
static SDL_Texture* g_texture = NULL;
static Uint32* g_pixels = NULL;
static sodna_Cell* g_cells = NULL;
static uint8_t* g_font = NULL;

static int g_columns;
static int g_rows;
static int g_font_w;
static int g_font_h;

const uint8_t font8[] = {
#include "font8.inc"
};

static inline uint8_t default_font_pixel(uint8_t symbol, int x, int y) {
    return font8[symbol * 8 + y] & (1 << (7 - x));
}

static inline size_t font_offset(uint8_t symbol, int x, int y) {
    return symbol * g_font_w * g_font_h + y * g_font_w + x;
}

static void init_font(uint8_t* font, int font_w, int font_h) {
    int c, x, y;
    for (c = 0; c < 256; c++) {
        for (y = 0; y < g_font_h; y++) {
            for (x = 0; x < g_font_w; x++) {
                font[font_offset(c, x, y)] = default_font_pixel(
                        c,
                        x * 8 / g_font_w,
                        y * 8 / g_font_h);
            }
        }
    }
}

static inline void cell_to_argb(Uint32* out_fore, Uint32* out_back, sodna_Cell cell) {
    *out_fore = 0xff000000 | cell.fore_r << 20 | cell.fore_g << 12 | cell.fore_b << 4;
    *out_back = 0xff000000 | cell.back_r << 20 | cell.back_g << 12 | cell.back_b << 4;
}

static int window_w() {
    return g_columns * g_font_w;
}

static int window_h() {
    return g_rows * g_font_h;
}

int sodna_init(
        int font_width, int font_height,
        int num_columns, int num_rows,
        const char* window_title) {
    g_font_w = font_width;
    g_font_h = font_height;
    g_columns = num_columns;
    g_rows = num_rows;

    // Already initialized.
    if (g_win)
        return 1;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return 1;

    g_win = SDL_CreateWindow(
            window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            window_w(), window_h(), SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!g_win)
        return 1;

    g_rend = SDL_CreateRenderer(g_win, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(g_rend, window_w(), window_h());

    g_texture = SDL_CreateTexture(
            g_rend, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            window_w(), window_h());

    g_pixels = (Uint32*)malloc(window_w() * window_h() * 4);
    g_cells = (sodna_Cell*)malloc(sodna_width() * sodna_height() * sizeof(sodna_Cell));
    memset(g_cells, 0, sodna_width() * sodna_height() * sizeof(sodna_Cell));
    g_font = (uint8_t*)malloc(g_font_w * g_font_h * 256);
    init_font(g_font, g_font_w, g_font_h);

    return 0;
}

void sodna_exit() {
    SDL_DestroyRenderer(g_rend); g_rend = NULL;
    SDL_DestroyWindow(g_win); g_win = NULL;
    SDL_DestroyTexture(g_texture); g_texture = NULL;
    free(g_pixels); g_pixels = NULL;
    free(g_cells); g_cells = NULL;
    free(g_font); g_font = NULL;
    SDL_Quit();
}

int sodna_font_width() { return g_font_w; }

int sodna_font_height() { return g_font_h; }

static void grab_char(uint8_t c, uint8_t* data, int pitch) {
    int x, y;
    for (y = 0; y < g_font_h; y++) {
        for (x = 0; x < g_font_w; x++) {
            g_font[font_offset(c, x, y)] = data[y * pitch + x];
        }
    }
}

void sodna_load_font_data(
        uint8_t* pixels,
        int pixels_width,
        int pixels_height,
        int first_char) {
    int x, y, c = first_char;
    int columns = pixels_width / g_font_w;
    int rows = pixels_height / g_font_h;
    if (columns <= 0 || rows <= 0)
        return;
    for (y = 0; y < rows; y++) {
        for (x = 0; x < columns; x++) {
            if (c >= 256)
                return;
            grab_char(c,
                    &pixels[y * g_font_h * pixels_width + x * g_font_w],
                    pixels_width);
            ++c;
        }
    }
}

sodna_Cell* sodna_cells() {
    return g_cells;
}

void draw_cell(int x, int y, Uint32 fore_col, Uint32 back_col, uint8_t symbol) {
    int u, v;
    for (v = 0; v < g_font_h; v++) {
        size_t offset = x + (y + v) * window_w();
        for (u = 0; u < g_font_w; u++) {
            // TODO: Interpolate colors on font grayscale.
            g_pixels[offset++] = g_font[font_offset(symbol, u, v)] ? fore_col : back_col;
        }
    }
}

void sodna_flush() {
    int x, y;
    sodna_Cell* cells = sodna_cells();
    /* XXX: Always repaints all cells, even if there was no change from
     * previous frame. Could use a twin cell buffer and check for change
     * from previous frame to see if we can skip draw_cell.
     */
    for (y = 0; y < sodna_height(); y++)
        for (x = 0; x < sodna_width(); x++) {
            Uint32 fore, back;
            sodna_Cell cell = cells[x + sodna_width() * y];
            cell_to_argb(&fore, &back, cell);
            draw_cell(x * g_font_w, y * g_font_h, fore, back, cell.symbol);
        }
    SDL_RenderClear(g_rend);
    SDL_UpdateTexture(g_texture, NULL, g_pixels, window_w() * sizeof(Uint32));
    SDL_RenderCopy(g_rend, g_texture, NULL, NULL);
    SDL_RenderPresent(g_rend);
}

int sodna_width() { return g_columns; }

int sodna_height() { return g_rows; }

static int process_event(SDL_Event* event) {
    int key = 0;
    if (event->type == SDL_WINDOWEVENT) {
        sodna_flush();
    }
    if (event->type == SDL_QUIT) {
        return SODNA_CLOSE_WINDOW;
    }

    /* Can't handle this event. */
    if (event->type != SDL_KEYDOWN)
        return 0;

    key = event->key.keysym.sym;
    /* Printable stuff. */
    if (key >= 32 && key < 128)
        return key;

    switch (key) {
        case SDLK_UP:
        case SDLK_KP_8:             return SODNA_UP;
        case SDLK_DOWN:
        case SDLK_KP_2:             return SODNA_DOWN;
        case SDLK_LEFT:
        case SDLK_KP_4:             return SODNA_LEFT;
        case SDLK_RIGHT:
        case SDLK_KP_6:             return SODNA_RIGHT;
        case SDLK_HOME:
        case SDLK_KP_7:             return SODNA_HOME;
        case SDLK_END:
        case SDLK_KP_1:             return SODNA_END;
        case SDLK_KP_5:             return SODNA_KP5;
        case SDLK_BACKSPACE:
        case SDLK_KP_BACKSPACE:     return SODNA_BACKSPACE;
        case SDLK_TAB:
        case SDLK_KP_TAB:           return SODNA_TAB;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:         return SODNA_ENTER;
        case SDLK_PAGEUP:
        case SDLK_KP_9:             return SODNA_PAGEUP;
        case SDLK_PAGEDOWN:
        case SDLK_KP_3:             return SODNA_PAGEDOWN;
        case SDLK_INSERT:           return SODNA_INSERT;
        case SDLK_DELETE:           return SODNA_DEL;
        case SDLK_F1:               return SODNA_F1;
        case SDLK_F2:               return SODNA_F2;
        case SDLK_F3:               return SODNA_F3;
        case SDLK_F4:               return SODNA_F4;
        case SDLK_F5:               return SODNA_F5;
        case SDLK_F6:               return SODNA_F6;
        case SDLK_F7:               return SODNA_F7;
        case SDLK_F8:               return SODNA_F8;
        case SDLK_F9:               return SODNA_F9;
        case SDLK_F10:              return SODNA_F10;
        case SDLK_F11:              return SODNA_F11;
        case SDLK_F12:              return SODNA_F12;
        case SDLK_ESCAPE:           return SODNA_ESC;
    }
    return 0;
}

int sodna_wait_event() {
    SDL_Event event;
    for (;;) {
        int ret;
        SDL_WaitEvent(&event);
        ret = process_event(&event);
        if (ret)
            return ret;
    }
    /* Shouldn't get here. */
    return 0;
}

int sodna_poll_event() {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        return process_event(&event);
    }
    return 0;
}

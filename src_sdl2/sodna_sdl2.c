#include "sodna.h"
#include <SDL.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <assert.h>

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

static size_t font_offset(uint8_t symbol, int x, int y) {
    return symbol * g_font_w * g_font_h + y * g_font_w + x;
}

static void cell_to_argb(Uint32* out_fore, Uint32* out_back, sodna_Cell cell) {
    *out_fore = 0xff000000 | cell.fore_r << 20 | cell.fore_g << 12 | cell.fore_b << 4;
    *out_back = 0xff000000 | cell.back_r << 20 | cell.back_g << 12 | cell.back_b << 4;
}

static void init_font(uint8_t* font, int font_w, int font_h) {
    int c, x, y;
    for (c = 0; c < 256; c++) {
        for (y = 0; y < font_h; y++) {
            for (x = 0; x < font_w; x++) {
                int src_x = x * 8 / font_w;
                int src_y = y * 8 / font_h;
                font[font_offset(c, x, y)] = font8[
                    ((c / 16) * 8 + src_y) * 8 * 16 +
                    (c % 16) * 8 + src_x];
            }
        }
    }
}

static int window_w() {
    return g_columns * g_font_w;
}

static int window_h() {
    return g_rows * g_font_h;
}

sodna_Error sodna_init(
        int font_width, int font_height,
        int num_columns, int num_rows,
        const char* window_title) {
    if (font_width < 1 || font_height < 1 || num_columns < 1 || num_rows < 1)
        return 1;

    /* Already initialized. */
    if (g_win)
        return SODNA_ERROR;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return SODNA_ERROR;

    g_win = SDL_CreateWindow(
            window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            font_width * num_columns, font_height * num_rows, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!g_win)
        return SODNA_ERROR;

    g_rend = SDL_CreateRenderer(g_win, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    sodna_resize(font_width, font_height, num_columns, num_rows);
    /* Simple aspect-retaining scaling, but not pixel-perfect. */
    /* SDL_RenderSetLogicalSize(g_rend, window_w(), window_h()); */

    return SODNA_OK;
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

void sodna_resize(int font_width, int font_height, int num_columns, int num_rows) {
    assert(font_width > 0 && font_height > 0 && num_columns > 0 && num_rows > 0);
    g_font_w = font_width;
    g_font_h = font_height;
    g_columns = num_columns;
    g_rows = num_rows;

    free(g_font); g_font = NULL;
    g_font = (uint8_t*)malloc(g_font_w * g_font_h * 256);
    init_font(g_font, g_font_w, g_font_h);

    SDL_DestroyTexture(g_texture); g_texture = NULL;
    g_texture = SDL_CreateTexture(
            g_rend, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            window_w(), window_h());

    free(g_pixels); g_pixels = NULL;
    g_pixels = (Uint32*)malloc(window_w() * window_h() * 4);

    free(g_cells); g_cells = NULL;
    g_cells = (sodna_Cell*)malloc(sodna_width() * sodna_height() * sizeof(sodna_Cell));
    memset(g_cells, 0, sodna_width() * sodna_height() * sizeof(sodna_Cell));
    SDL_SetWindowSize(g_win, window_w(), window_h());
}

static void grab_char(uint8_t c, const uint8_t* data, int pitch) {
    int x, y;
    for (y = 0; y < g_font_h; y++) {
        for (x = 0; x < g_font_w; x++) {
            g_font[font_offset(c, x, y)] = data[y * pitch + x];
        }
    }
}

void sodna_load_font_data(
        const uint8_t* pixels,
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

void sodna_set_edge_color(int color) {
    SDL_SetRenderDrawColor(g_rend, (color >> 8) << 4 , color & 0xf0, (color % 16) << 4, 255);
}

void draw_cell(int x, int y, Uint32 fore_col, Uint32 back_col, uint8_t symbol) {
    int u, v;
    for (v = 0; v < g_font_h; v++) {
        size_t offset = x + (y + v) * window_w();
        for (u = 0; u < g_font_w; u++) {
            uint8_t col = g_font[font_offset(symbol, u, v)];
            int i, b, f;
            switch (col) {
                case 0:
                    g_pixels[offset++] = back_col;
                    break;
                default:
                    /* Interpolate between background and foreground. */
                    g_pixels[offset] = 0;
                    for (i = 0; i < 32; i += 8) {
                        b = (back_col >> i) % 0xff;
                        f = (fore_col >> i) % 0xff;
                        g_pixels[offset] |= (b + (f - b) * col / 255) << i;
                    }
                    ++offset;
                    break;
                case 255:
                    g_pixels[offset++] = fore_col;
                    break;
            }
        }
    }
}

static void pixel_perfect_target_rect(
        SDL_Rect* out_rect, int w, int h, SDL_Renderer* rend) {
    SDL_Rect viewport;
    float w_scale, h_scale, min_scale;
    memset(out_rect, 0, sizeof(SDL_Rect));
    SDL_RenderGetViewport(rend, &viewport);

    w_scale = (float)viewport.w / w;
    h_scale = (float)viewport.h / h;
    min_scale = w_scale < h_scale ? w_scale : h_scale;

    if (min_scale < 1.f) {
        /* Less than 1 physical pixel for 1 logical pixel, can't
         * achieve pixel perfection so just scale to what we get.
         */
        out_rect->w = w * min_scale;
        out_rect->h = h * min_scale;
    } else {
        out_rect->w = w * floor(min_scale);
        out_rect->h = h * floor(min_scale);
    }
    out_rect->x = (viewport.w - out_rect->w) / 2;
    out_rect->y = (viewport.h - out_rect->h) / 2;
}

void sodna_flush() {
    int x, y;
    SDL_Rect target;
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

    pixel_perfect_target_rect(&target, window_w(), window_h(), g_rend);
    SDL_RenderCopy(g_rend, g_texture, NULL, &target);
    SDL_RenderPresent(g_rend);
}

int sodna_width() { return g_columns; }

int sodna_height() { return g_rows; }

/* Map screen coordinates to character cell coordinates. Return whether the
 * resulting cell is within the screen cell array.
 */
static int mouse_pos_to_cells(int* x, int* y) {
    SDL_Rect target;
    pixel_perfect_target_rect(&target, window_w(), window_h(), g_rend);
    *x -= target.x;
    *y -= target.y;
    *x /= target.w / g_columns;
    *y /= target.h / g_rows;
    return *x >= 0 && *y >= 0 && *x < window_w() && *y < window_h();
}

static sodna_Event add_mouse_button(sodna_Event event, const SDL_Event* sdl_event) {
    switch (sdl_event->button.button) {
        case SDL_BUTTON_LEFT:
            event.button.id = SODNA_LEFT_BUTTON;
            break;
        case SDL_BUTTON_MIDDLE:
            event.button.id = SODNA_MIDDLE_BUTTON;
            break;
        case SDL_BUTTON_RIGHT:
            event.button.id = SODNA_RIGHT_BUTTON;
            break;
    }
    return event;
}

#define SCANCODE_TABLE \
    X(SDL_SCANCODE_A, SODNA_KEY_A) \
    X(SDL_SCANCODE_B, SODNA_KEY_B) \
    X(SDL_SCANCODE_C, SODNA_KEY_C) \
    X(SDL_SCANCODE_D, SODNA_KEY_D) \
    X(SDL_SCANCODE_E, SODNA_KEY_E) \
    X(SDL_SCANCODE_F, SODNA_KEY_F) \
    X(SDL_SCANCODE_G, SODNA_KEY_G) \
    X(SDL_SCANCODE_H, SODNA_KEY_H) \
    X(SDL_SCANCODE_I, SODNA_KEY_I) \
    X(SDL_SCANCODE_J, SODNA_KEY_J) \
    X(SDL_SCANCODE_K, SODNA_KEY_K) \
    X(SDL_SCANCODE_L, SODNA_KEY_L) \
    X(SDL_SCANCODE_M, SODNA_KEY_M) \
    X(SDL_SCANCODE_N, SODNA_KEY_N) \
    X(SDL_SCANCODE_O, SODNA_KEY_O) \
    X(SDL_SCANCODE_P, SODNA_KEY_P) \
    X(SDL_SCANCODE_Q, SODNA_KEY_Q) \
    X(SDL_SCANCODE_R, SODNA_KEY_R) \
    X(SDL_SCANCODE_S, SODNA_KEY_S) \
    X(SDL_SCANCODE_T, SODNA_KEY_T) \
    X(SDL_SCANCODE_U, SODNA_KEY_U) \
    X(SDL_SCANCODE_V, SODNA_KEY_V) \
    X(SDL_SCANCODE_W, SODNA_KEY_W) \
    X(SDL_SCANCODE_X, SODNA_KEY_X) \
    X(SDL_SCANCODE_Y, SODNA_KEY_Y) \
    X(SDL_SCANCODE_Z, SODNA_KEY_Z) \
    X(SDL_SCANCODE_1, SODNA_KEY_1) \
    X(SDL_SCANCODE_2, SODNA_KEY_2) \
    X(SDL_SCANCODE_3, SODNA_KEY_3) \
    X(SDL_SCANCODE_4, SODNA_KEY_4) \
    X(SDL_SCANCODE_5, SODNA_KEY_5) \
    X(SDL_SCANCODE_6, SODNA_KEY_6) \
    X(SDL_SCANCODE_7, SODNA_KEY_7) \
    X(SDL_SCANCODE_8, SODNA_KEY_8) \
    X(SDL_SCANCODE_9, SODNA_KEY_9) \
    X(SDL_SCANCODE_0, SODNA_KEY_0) \
    X(SDL_SCANCODE_RETURN, SODNA_KEY_ENTER) \
    X(SDL_SCANCODE_ESCAPE, SODNA_KEY_ESCAPE) \
    X(SDL_SCANCODE_BACKSPACE, SODNA_KEY_BACKSPACE) \
    X(SDL_SCANCODE_TAB, SODNA_KEY_TAB) \
    X(SDL_SCANCODE_SPACE, SODNA_KEY_SPACE) \
    X(SDL_SCANCODE_MINUS, SODNA_KEY_MINUS) \
    X(SDL_SCANCODE_EQUALS, SODNA_KEY_EQUALS) \
    X(SDL_SCANCODE_LEFTBRACKET, SODNA_KEY_LEFTBRACKET) \
    X(SDL_SCANCODE_RIGHTBRACKET, SODNA_KEY_RIGHTBRACKET) \
    X(SDL_SCANCODE_BACKSLASH, SODNA_KEY_BACKSLASH) \
    X(SDL_SCANCODE_SEMICOLON, SODNA_KEY_SEMICOLON) \
    X(SDL_SCANCODE_APOSTROPHE, SODNA_KEY_APOSTROPHE) \
    X(SDL_SCANCODE_GRAVE, SODNA_KEY_GRAVE) \
    X(SDL_SCANCODE_COMMA, SODNA_KEY_COMMA) \
    X(SDL_SCANCODE_PERIOD, SODNA_KEY_PERIOD) \
    X(SDL_SCANCODE_SLASH, SODNA_KEY_SLASH) \
    X(SDL_SCANCODE_CAPSLOCK, SODNA_KEY_CAPS_LOCK) \
    X(SDL_SCANCODE_F1, SODNA_KEY_F1) \
    X(SDL_SCANCODE_F2, SODNA_KEY_F2) \
    X(SDL_SCANCODE_F3, SODNA_KEY_F3) \
    X(SDL_SCANCODE_F4, SODNA_KEY_F4) \
    X(SDL_SCANCODE_F5, SODNA_KEY_F5) \
    X(SDL_SCANCODE_F6, SODNA_KEY_F6) \
    X(SDL_SCANCODE_F7, SODNA_KEY_F7) \
    X(SDL_SCANCODE_F8, SODNA_KEY_F8) \
    X(SDL_SCANCODE_F9, SODNA_KEY_F9) \
    X(SDL_SCANCODE_F10, SODNA_KEY_F10) \
    X(SDL_SCANCODE_F11, SODNA_KEY_F11) \
    X(SDL_SCANCODE_F12, SODNA_KEY_F12) \
    X(SDL_SCANCODE_PRINTSCREEN, SODNA_KEY_PRINT_SCREEN) \
    X(SDL_SCANCODE_SCROLLLOCK, SODNA_KEY_SCROLL_LOCK) \
    X(SDL_SCANCODE_PAUSE, SODNA_KEY_PAUSE) \
    X(SDL_SCANCODE_INSERT, SODNA_KEY_INSERT) \
    X(SDL_SCANCODE_HOME, SODNA_KEY_HOME) \
    X(SDL_SCANCODE_PAGEUP, SODNA_KEY_PAGE_UP) \
    X(SDL_SCANCODE_DELETE, SODNA_KEY_DELETE) \
    X(SDL_SCANCODE_END, SODNA_KEY_END) \
    X(SDL_SCANCODE_PAGEDOWN, SODNA_KEY_PAGE_DOWN) \
    X(SDL_SCANCODE_RIGHT, SODNA_KEY_RIGHT) \
    X(SDL_SCANCODE_LEFT, SODNA_KEY_LEFT) \
    X(SDL_SCANCODE_DOWN, SODNA_KEY_DOWN) \
    X(SDL_SCANCODE_UP, SODNA_KEY_UP) \
    X(SDL_SCANCODE_NUMLOCKCLEAR, SODNA_KEY_NUM_LOCK) \
    X(SDL_SCANCODE_KP_MULTIPLY, SODNA_KEY_KP_MULTIPLY) \
    X(SDL_SCANCODE_KP_MINUS, SODNA_KEY_KP_MINUS) \
    X(SDL_SCANCODE_KP_PLUS, SODNA_KEY_KP_PLUS) \
    X(SDL_SCANCODE_KP_ENTER, SODNA_KEY_KP_ENTER) \
    X(SDL_SCANCODE_KP_1, SODNA_KEY_KP_1) \
    X(SDL_SCANCODE_KP_2, SODNA_KEY_KP_2) \
    X(SDL_SCANCODE_KP_3, SODNA_KEY_KP_3) \
    X(SDL_SCANCODE_KP_4, SODNA_KEY_KP_4) \
    X(SDL_SCANCODE_KP_5, SODNA_KEY_KP_5) \
    X(SDL_SCANCODE_KP_6, SODNA_KEY_KP_6) \
    X(SDL_SCANCODE_KP_7, SODNA_KEY_KP_7) \
    X(SDL_SCANCODE_KP_8, SODNA_KEY_KP_8) \
    X(SDL_SCANCODE_KP_9, SODNA_KEY_KP_9) \
    X(SDL_SCANCODE_KP_0, SODNA_KEY_KP_0) \
    X(SDL_SCANCODE_KP_PERIOD, SODNA_KEY_KP_DECIMAL) \
    X(SDL_SCANCODE_KP_EQUALS, SODNA_KEY_KP_EQUALS) \
    X(SDL_SCANCODE_LCTRL, SODNA_KEY_LEFT_CONTROL) \
    X(SDL_SCANCODE_LSHIFT, SODNA_KEY_LEFT_SHIFT) \
    X(SDL_SCANCODE_LALT, SODNA_KEY_LEFT_ALT) \
    X(SDL_SCANCODE_LGUI, SODNA_KEY_LEFT_SUPER) \
    X(SDL_SCANCODE_RCTRL, SODNA_KEY_RIGHT_CONTROL) \
    X(SDL_SCANCODE_RSHIFT, SODNA_KEY_RIGHT_SHIFT) \
    X(SDL_SCANCODE_RALT, SODNA_KEY_RIGHT_ALT) \
    X(SDL_SCANCODE_RGUI, SODNA_KEY_RIGHT_SUPER)

#define KEYSYM_TABLE \
    X(SDLK_a, SODNA_KEY_A) \
    X(SDLK_b, SODNA_KEY_B) \
    X(SDLK_c, SODNA_KEY_C) \
    X(SDLK_d, SODNA_KEY_D) \
    X(SDLK_e, SODNA_KEY_E) \
    X(SDLK_f, SODNA_KEY_F) \
    X(SDLK_g, SODNA_KEY_G) \
    X(SDLK_h, SODNA_KEY_H) \
    X(SDLK_i, SODNA_KEY_I) \
    X(SDLK_j, SODNA_KEY_J) \
    X(SDLK_k, SODNA_KEY_K) \
    X(SDLK_l, SODNA_KEY_L) \
    X(SDLK_m, SODNA_KEY_M) \
    X(SDLK_n, SODNA_KEY_N) \
    X(SDLK_o, SODNA_KEY_O) \
    X(SDLK_p, SODNA_KEY_P) \
    X(SDLK_q, SODNA_KEY_Q) \
    X(SDLK_r, SODNA_KEY_R) \
    X(SDLK_s, SODNA_KEY_S) \
    X(SDLK_t, SODNA_KEY_T) \
    X(SDLK_u, SODNA_KEY_U) \
    X(SDLK_v, SODNA_KEY_V) \
    X(SDLK_w, SODNA_KEY_W) \
    X(SDLK_x, SODNA_KEY_X) \
    X(SDLK_y, SODNA_KEY_Y) \
    X(SDLK_z, SODNA_KEY_Z) \
    X(SDLK_1, SODNA_KEY_1) \
    X(SDLK_2, SODNA_KEY_2) \
    X(SDLK_3, SODNA_KEY_3) \
    X(SDLK_4, SODNA_KEY_4) \
    X(SDLK_5, SODNA_KEY_5) \
    X(SDLK_6, SODNA_KEY_6) \
    X(SDLK_7, SODNA_KEY_7) \
    X(SDLK_8, SODNA_KEY_8) \
    X(SDLK_9, SODNA_KEY_9) \
    X(SDLK_0, SODNA_KEY_0) \
    X(SDLK_RETURN, SODNA_KEY_ENTER) \
    X(SDLK_ESCAPE, SODNA_KEY_ESCAPE) \
    X(SDLK_BACKSPACE, SODNA_KEY_BACKSPACE) \
    X(SDLK_TAB, SODNA_KEY_TAB) \
    X(SDLK_SPACE, SODNA_KEY_SPACE) \
    X(SDLK_MINUS, SODNA_KEY_MINUS) \
    X(SDLK_EQUALS, SODNA_KEY_EQUALS) \
    X(SDLK_LEFTBRACKET, SODNA_KEY_LEFTBRACKET) \
    X(SDLK_RIGHTBRACKET, SODNA_KEY_RIGHTBRACKET) \
    X(SDLK_BACKSLASH, SODNA_KEY_BACKSLASH) \
    X(SDLK_SEMICOLON, SODNA_KEY_SEMICOLON) \
    X(SDLK_QUOTE, SODNA_KEY_APOSTROPHE) \
    X(SDLK_BACKQUOTE, SODNA_KEY_GRAVE) \
    X(SDLK_COMMA, SODNA_KEY_COMMA) \
    X(SDLK_PERIOD, SODNA_KEY_PERIOD) \
    X(SDLK_SLASH, SODNA_KEY_SLASH) \
    X(SDLK_CAPSLOCK, SODNA_KEY_CAPS_LOCK) \
    X(SDLK_F1, SODNA_KEY_F1) \
    X(SDLK_F2, SODNA_KEY_F2) \
    X(SDLK_F3, SODNA_KEY_F3) \
    X(SDLK_F4, SODNA_KEY_F4) \
    X(SDLK_F5, SODNA_KEY_F5) \
    X(SDLK_F6, SODNA_KEY_F6) \
    X(SDLK_F7, SODNA_KEY_F7) \
    X(SDLK_F8, SODNA_KEY_F8) \
    X(SDLK_F9, SODNA_KEY_F9) \
    X(SDLK_F10, SODNA_KEY_F10) \
    X(SDLK_F11, SODNA_KEY_F11) \
    X(SDLK_F12, SODNA_KEY_F12) \
    X(SDLK_PRINTSCREEN, SODNA_KEY_PRINT_SCREEN) \
    X(SDLK_SCROLLLOCK, SODNA_KEY_SCROLL_LOCK) \
    X(SDLK_PAUSE, SODNA_KEY_PAUSE) \
    X(SDLK_INSERT, SODNA_KEY_INSERT) \
    X(SDLK_HOME, SODNA_KEY_HOME) \
    X(SDLK_PAGEUP, SODNA_KEY_PAGE_UP) \
    X(SDLK_DELETE, SODNA_KEY_DELETE) \
    X(SDLK_END, SODNA_KEY_END) \
    X(SDLK_PAGEDOWN, SODNA_KEY_PAGE_DOWN) \
    X(SDLK_RIGHT, SODNA_KEY_RIGHT) \
    X(SDLK_LEFT, SODNA_KEY_LEFT) \
    X(SDLK_DOWN, SODNA_KEY_DOWN) \
    X(SDLK_UP, SODNA_KEY_UP) \
    X(SDLK_NUMLOCKCLEAR, SODNA_KEY_NUM_LOCK) \
    X(SDLK_KP_MULTIPLY, SODNA_KEY_KP_MULTIPLY) \
    X(SDLK_KP_MINUS, SODNA_KEY_KP_MINUS) \
    X(SDLK_KP_PLUS, SODNA_KEY_KP_PLUS) \
    X(SDLK_KP_ENTER, SODNA_KEY_KP_ENTER) \
    X(SDLK_KP_1, SODNA_KEY_KP_1) \
    X(SDLK_KP_2, SODNA_KEY_KP_2) \
    X(SDLK_KP_3, SODNA_KEY_KP_3) \
    X(SDLK_KP_4, SODNA_KEY_KP_4) \
    X(SDLK_KP_5, SODNA_KEY_KP_5) \
    X(SDLK_KP_6, SODNA_KEY_KP_6) \
    X(SDLK_KP_7, SODNA_KEY_KP_7) \
    X(SDLK_KP_8, SODNA_KEY_KP_8) \
    X(SDLK_KP_9, SODNA_KEY_KP_9) \
    X(SDLK_KP_0, SODNA_KEY_KP_0) \
    X(SDLK_KP_PERIOD, SODNA_KEY_KP_DECIMAL) \
    X(SDLK_KP_EQUALS, SODNA_KEY_KP_EQUALS) \
    X(SDLK_LCTRL, SODNA_KEY_LEFT_CONTROL) \
    X(SDLK_LSHIFT, SODNA_KEY_LEFT_SHIFT) \
    X(SDLK_LALT, SODNA_KEY_LEFT_ALT) \
    X(SDLK_LGUI, SODNA_KEY_LEFT_SUPER) \
    X(SDLK_RCTRL, SODNA_KEY_RIGHT_CONTROL) \
    X(SDLK_RSHIFT, SODNA_KEY_RIGHT_SHIFT) \
    X(SDLK_RALT, SODNA_KEY_RIGHT_ALT) \
    X(SDLK_RGUI, SODNA_KEY_RIGHT_SUPER)

static sodna_Event process_event(const SDL_Event* event) {
    sodna_Event ret;
    memset(&ret, 0, sizeof(ret));

    if (event->type == SDL_WINDOWEVENT) {
        sodna_flush();
        switch (event->window.event) {
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                ret.type = SODNA_EVENT_FOCUS_GAINED;
                return ret;
            case SDL_WINDOWEVENT_LEAVE:
            case SDL_WINDOWEVENT_FOCUS_LOST:
                ret.type = SODNA_EVENT_FOCUS_LOST;
                return ret;
        }
        return ret;
    }

    if (event->type == SDL_QUIT) {
        ret.type = SODNA_EVENT_CLOSE_WINDOW;
        return ret;
    }

    if (event->type == SDL_MOUSEMOTION) {
        int x = event->motion.x, y = event->motion.y;
        if (!mouse_pos_to_cells(&x, &y))
            return ret;
        ret.type = SODNA_EVENT_MOUSE_MOVED;
        ret.mouse.x = x;
        ret.mouse.y = y;
        return ret;
    }

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        ret.type = SODNA_EVENT_MOUSE_DOWN;
        return add_mouse_button(ret, event);
    }

    if (event->type == SDL_MOUSEBUTTONUP) {
        ret.type = SODNA_EVENT_MOUSE_UP;
        return add_mouse_button(ret, event);
    }

    if (event->type == SDL_MOUSEWHEEL) {
        ret.type = SODNA_EVENT_MOUSE_WHEEL;
        ret.wheel.delta = event->wheel.y;
        return ret;
    }

    if (event->type == SDL_TEXTINPUT) {
        ret.type = SODNA_EVENT_CHARACTER;
        // TODO: This only handles ASCII-7. Assume text is UTF-8, read the
        // first codepoint into ch.code.
        ret.ch.code = event->text.text[0];
        return ret;
    }

    if (event->type == SDL_KEYDOWN)
        ret.type = SODNA_EVENT_KEY_DOWN;
    else if (event->type == SDL_KEYUP)
        ret.type = SODNA_EVENT_KEY_UP;
    else
        /* Can't handle the rest of types, bail out. */
        return ret;

    switch (event->key.keysym.sym) {
#define X(sdl, sodna) case sdl: ret.key.layout = sodna; break;
        KEYSYM_TABLE
#undef X
        default:
            ret.key.layout = SODNA_KEY_UNKNOWN;
            break;
    }

    switch (event->key.keysym.scancode) {
#define X(sdl, sodna) case sdl: ret.key.hardware = sodna; break;
        SCANCODE_TABLE
#undef X
        default:
            ret.key.hardware = SODNA_KEY_UNKNOWN;
            break;
    }

    return ret;
}

sodna_Event sodna_wait_event(int timeout_ms) {
    SDL_Event event;
    int start_time = SDL_GetTicks();
    for (;;) {
        int status;
        sodna_Event ret;
        memset(&ret, 0, sizeof(ret));

        if (timeout_ms <= 0)
            status = SDL_WaitEvent(&event);
        else
            status = SDL_WaitEventTimeout(
                    &event, timeout_ms - (SDL_GetTicks() - start_time));
        if (status == 0)
            return ret;
        ret = process_event(&event);
        if (ret.type)
            return ret;
    }
}

sodna_Event sodna_poll_event() {
    static sodna_Event empty;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        sodna_Event ret = process_event(&event);
        if (ret.type)
            return ret;
    }
    return empty;
}

int sodna_ms_elapsed() {
    return SDL_GetTicks();
}

sodna_Error sodna_sleep_ms(int ms) {
    SDL_Delay(ms);
    return SODNA_OK;
}

sodna_Error sodna_save_screenshot(const char* path) {
    int i;
    Uint32* buffer = (Uint32*)malloc(window_w() * window_h() * 4);
    for (i = 0; i < window_w() * window_h(); i++) {
        Uint8 r, g, b;
        r = g_pixels[i] >> 16;
        g = g_pixels[i] >> 8;
        b = g_pixels[i];
        buffer[i] = 0xff000000 | b << 16 | g << 8 | r;
    }
    int ret = stbi_write_png(path, window_w(), window_h(), 4, buffer, 0);
    free(buffer);
    return ret ? SODNA_ERROR : SODNA_OK;
}

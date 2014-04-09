#include "sodna.h"
#include <SDL.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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

    g_font_w = font_width;
    g_font_h = font_height;
    g_columns = num_columns;
    g_rows = num_rows;

    /* Already initialized. */
    if (g_win)
        return SODNA_ERROR;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return SODNA_ERROR;

    g_win = SDL_CreateWindow(
            window_title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            window_w(), window_h(), SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!g_win)
        return SODNA_ERROR;

    g_rend = SDL_CreateRenderer(g_win, -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    /* Simple aspect-retaining scaling, but not pixel-perfect. */
    /* SDL_RenderSetLogicalSize(g_rend, window_w(), window_h()); */

    g_texture = SDL_CreateTexture(
            g_rend, SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            window_w(), window_h());

    g_pixels = (Uint32*)malloc(window_w() * window_h() * 4);
    g_cells = (sodna_Cell*)malloc(sodna_width() * sodna_height() * sizeof(sodna_Cell));
    memset(g_cells, 0, sodna_width() * sodna_height() * sizeof(sodna_Cell));
    g_font = (uint8_t*)malloc(g_font_w * g_font_h * 256);
    init_font(g_font, g_font_w, g_font_h);

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

static int add_mouse_pos(int event, int x, int y) {
    if (!mouse_pos_to_cells(&x, &y))
        return 0;
    int coords = (x & 0xfff) << 7 | (y & 0xfff) << 19;
    return (event & 0x8000007f) | coords;
}

static int add_mouse_button(int event, const SDL_Event* sdl_event) {
    switch (sdl_event->button.button) {
        case SDL_BUTTON_LEFT:
            return event | (SODNA_LEFT_BUTTON << 7);
        case SDL_BUTTON_MIDDLE:
            return event | (SODNA_MIDDLE_BUTTON << 7);
        case SDL_BUTTON_RIGHT:
            return event | (SODNA_RIGHT_BUTTON << 7);
    }
    return event;
}

static int add_mouse_wheel(int event, int delta) {
    return event | ((delta & 0x2) << 7);
}

/* Non-character event structure:
 * Negative numbers, first seven bits designate event IDs.
 * The next 24 bits are mouse x and y positions for mouse events.
 *
 * low bits ------------------------> high bits
 * --- 7 ---|--- 12 ---|--- 12 ---|---- 1 ----|
 * event id | mouse x  | mouse y  | minus bit |
 */
static int process_event(const SDL_Event* event) {
    int scan = 0;
    int sign = 1;

    if (event->type == SDL_WINDOWEVENT) {
        sodna_flush();
        switch (event->window.event) {
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                return SODNA_FOCUS_GAINED;
            case SDL_WINDOWEVENT_LEAVE:
            case SDL_WINDOWEVENT_FOCUS_LOST:
                return SODNA_FOCUS_LOST;
        }
        return 0;
    }

    if (event->type == SDL_QUIT) {
        return SODNA_CLOSE_WINDOW;
    }

    if (event->type == SDL_MOUSEMOTION) {
        int result = SODNA_MOUSE_MOVED;
        result = add_mouse_pos(result, event->motion.x, event->motion.y);
        return result;
    }

    if (event->type == SDL_MOUSEBUTTONDOWN) {
        int result = SODNA_MOUSE_DOWN;
        return add_mouse_button(result, event);
        return result;
    }

    if (event->type == SDL_MOUSEBUTTONUP) {
        int result = SODNA_MOUSE_UP;
        return add_mouse_button(result, event);
    }

    if (event->type == SDL_MOUSEWHEEL) {
        int result = SODNA_MOUSE_WHEEL;
        return add_mouse_wheel(result, event->wheel.y);
    }

    if (event->type == SDL_TEXTINPUT) {
        // TODO: Handle unicode input.
        return event->text.text[0];
    }

    /* Can't handle this event. */
    if (event->type != SDL_KEYDOWN && event->type != SDL_KEYUP)
        return 0;

    if (event->type == SDL_KEYUP)
        sign = -1;

    scan = event->key.keysym.scancode;
#define K(sdl, sodna) case sdl: return ((Uint8)(Sint8)(sodna * sign)) << 16
    switch (scan) {
        K(SDL_SCANCODE_A, SODNA_SCANCODE_A);
        K(SDL_SCANCODE_B, SODNA_SCANCODE_B);
        K(SDL_SCANCODE_C, SODNA_SCANCODE_C);
        K(SDL_SCANCODE_D, SODNA_SCANCODE_D);
        K(SDL_SCANCODE_E, SODNA_SCANCODE_E);
        K(SDL_SCANCODE_F, SODNA_SCANCODE_F);
        K(SDL_SCANCODE_G, SODNA_SCANCODE_G);
        K(SDL_SCANCODE_H, SODNA_SCANCODE_H);
        K(SDL_SCANCODE_I, SODNA_SCANCODE_I);
        K(SDL_SCANCODE_J, SODNA_SCANCODE_J);
        K(SDL_SCANCODE_K, SODNA_SCANCODE_K);
        K(SDL_SCANCODE_L, SODNA_SCANCODE_L);
        K(SDL_SCANCODE_M, SODNA_SCANCODE_M);
        K(SDL_SCANCODE_N, SODNA_SCANCODE_N);
        K(SDL_SCANCODE_O, SODNA_SCANCODE_O);
        K(SDL_SCANCODE_P, SODNA_SCANCODE_P);
        K(SDL_SCANCODE_Q, SODNA_SCANCODE_Q);
        K(SDL_SCANCODE_R, SODNA_SCANCODE_R);
        K(SDL_SCANCODE_S, SODNA_SCANCODE_S);
        K(SDL_SCANCODE_T, SODNA_SCANCODE_T);
        K(SDL_SCANCODE_U, SODNA_SCANCODE_U);
        K(SDL_SCANCODE_V, SODNA_SCANCODE_V);
        K(SDL_SCANCODE_W, SODNA_SCANCODE_W);
        K(SDL_SCANCODE_X, SODNA_SCANCODE_X);
        K(SDL_SCANCODE_Y, SODNA_SCANCODE_Y);
        K(SDL_SCANCODE_Z, SODNA_SCANCODE_Z);
        K(SDL_SCANCODE_1, SODNA_SCANCODE_1);
        K(SDL_SCANCODE_2, SODNA_SCANCODE_2);
        K(SDL_SCANCODE_3, SODNA_SCANCODE_3);
        K(SDL_SCANCODE_4, SODNA_SCANCODE_4);
        K(SDL_SCANCODE_5, SODNA_SCANCODE_5);
        K(SDL_SCANCODE_6, SODNA_SCANCODE_6);
        K(SDL_SCANCODE_7, SODNA_SCANCODE_7);
        K(SDL_SCANCODE_8, SODNA_SCANCODE_8);
        K(SDL_SCANCODE_9, SODNA_SCANCODE_9);
        K(SDL_SCANCODE_0, SODNA_SCANCODE_0);
        K(SDL_SCANCODE_RETURN, SODNA_SCANCODE_ENTER);
        K(SDL_SCANCODE_ESCAPE, SODNA_SCANCODE_ESCAPE);
        K(SDL_SCANCODE_BACKSPACE, SODNA_SCANCODE_BACKSPACE);
        K(SDL_SCANCODE_TAB, SODNA_SCANCODE_TAB);
        K(SDL_SCANCODE_SPACE, SODNA_SCANCODE_SPACE);
        K(SDL_SCANCODE_MINUS, SODNA_SCANCODE_MINUS);
        K(SDL_SCANCODE_EQUALS, SODNA_SCANCODE_EQUALS);
        K(SDL_SCANCODE_LEFTBRACKET, SODNA_SCANCODE_LEFTBRACKET);
        K(SDL_SCANCODE_RIGHTBRACKET, SODNA_SCANCODE_RIGHTBRACKET);
        K(SDL_SCANCODE_BACKSLASH, SODNA_SCANCODE_BACKSLASH);
        K(SDL_SCANCODE_SEMICOLON, SODNA_SCANCODE_SEMICOLON);
        K(SDL_SCANCODE_APOSTROPHE, SODNA_SCANCODE_APOSTROPHE);
        K(SDL_SCANCODE_GRAVE, SODNA_SCANCODE_GRAVE);
        K(SDL_SCANCODE_COMMA, SODNA_SCANCODE_COMMA);
        K(SDL_SCANCODE_PERIOD, SODNA_SCANCODE_PERIOD);
        K(SDL_SCANCODE_SLASH, SODNA_SCANCODE_SLASH);
        K(SDL_SCANCODE_CAPSLOCK, SODNA_SCANCODE_CAPS_LOCK);
        K(SDL_SCANCODE_F1, SODNA_SCANCODE_F1);
        K(SDL_SCANCODE_F2, SODNA_SCANCODE_F2);
        K(SDL_SCANCODE_F3, SODNA_SCANCODE_F3);
        K(SDL_SCANCODE_F4, SODNA_SCANCODE_F4);
        K(SDL_SCANCODE_F5, SODNA_SCANCODE_F5);
        K(SDL_SCANCODE_F6, SODNA_SCANCODE_F6);
        K(SDL_SCANCODE_F7, SODNA_SCANCODE_F7);
        K(SDL_SCANCODE_F8, SODNA_SCANCODE_F8);
        K(SDL_SCANCODE_F9, SODNA_SCANCODE_F9);
        K(SDL_SCANCODE_F10, SODNA_SCANCODE_F10);
        K(SDL_SCANCODE_F11, SODNA_SCANCODE_F11);
        K(SDL_SCANCODE_F12, SODNA_SCANCODE_F12);
        K(SDL_SCANCODE_PRINTSCREEN, SODNA_SCANCODE_PRINT_SCREEN);
        K(SDL_SCANCODE_SCROLLLOCK, SODNA_SCANCODE_SCROLL_LOCK);
        K(SDL_SCANCODE_PAUSE, SODNA_SCANCODE_PAUSE);
        K(SDL_SCANCODE_INSERT, SODNA_SCANCODE_INSERT);
        K(SDL_SCANCODE_HOME, SODNA_SCANCODE_HOME);
        K(SDL_SCANCODE_PAGEUP, SODNA_SCANCODE_PAGE_UP);
        K(SDL_SCANCODE_DELETE, SODNA_SCANCODE_DELETE);
        K(SDL_SCANCODE_END, SODNA_SCANCODE_END);
        K(SDL_SCANCODE_PAGEDOWN, SODNA_SCANCODE_PAGE_DOWN);
        K(SDL_SCANCODE_RIGHT, SODNA_SCANCODE_RIGHT);
        K(SDL_SCANCODE_LEFT, SODNA_SCANCODE_LEFT);
        K(SDL_SCANCODE_DOWN, SODNA_SCANCODE_DOWN);
        K(SDL_SCANCODE_UP, SODNA_SCANCODE_UP);
        K(SDL_SCANCODE_NUMLOCKCLEAR, SODNA_SCANCODE_NUM_LOCK);
        K(SDL_SCANCODE_KP_MULTIPLY, SODNA_SCANCODE_KP_MULTIPLY);
        K(SDL_SCANCODE_KP_MINUS, SODNA_SCANCODE_KP_MINUS);
        K(SDL_SCANCODE_KP_PLUS, SODNA_SCANCODE_KP_PLUS);
        K(SDL_SCANCODE_KP_ENTER, SODNA_SCANCODE_KP_ENTER);
        K(SDL_SCANCODE_KP_1, SODNA_SCANCODE_KP_1);
        K(SDL_SCANCODE_KP_2, SODNA_SCANCODE_KP_2);
        K(SDL_SCANCODE_KP_3, SODNA_SCANCODE_KP_3);
        K(SDL_SCANCODE_KP_4, SODNA_SCANCODE_KP_4);
        K(SDL_SCANCODE_KP_5, SODNA_SCANCODE_KP_5);
        K(SDL_SCANCODE_KP_6, SODNA_SCANCODE_KP_6);
        K(SDL_SCANCODE_KP_7, SODNA_SCANCODE_KP_7);
        K(SDL_SCANCODE_KP_8, SODNA_SCANCODE_KP_8);
        K(SDL_SCANCODE_KP_9, SODNA_SCANCODE_KP_9);
        K(SDL_SCANCODE_KP_0, SODNA_SCANCODE_KP_0);
        K(SDL_SCANCODE_KP_PERIOD, SODNA_SCANCODE_KP_DECIMAL);
        K(SDL_SCANCODE_KP_EQUALS, SODNA_SCANCODE_KP_EQUALS);
        K(SDL_SCANCODE_LCTRL, SODNA_SCANCODE_LEFT_CONTROL);
        K(SDL_SCANCODE_LSHIFT, SODNA_SCANCODE_LEFT_SHIFT);
        K(SDL_SCANCODE_LALT, SODNA_SCANCODE_LEFT_ALT);
        K(SDL_SCANCODE_LGUI, SODNA_SCANCODE_LEFT_SUPER);
        K(SDL_SCANCODE_RCTRL, SODNA_SCANCODE_RIGHT_CONTROL);
        K(SDL_SCANCODE_RSHIFT, SODNA_SCANCODE_RIGHT_SHIFT);
        K(SDL_SCANCODE_RALT, SODNA_SCANCODE_RIGHT_ALT);
        K(SDL_SCANCODE_RGUI, SODNA_SCANCODE_RIGHT_SUPER);
        default: return ((Uint8)(SODNA_SCANCODE_UNKNOWN * sign)) << 16;
    }
}

int sodna_wait_event(int timeout_ms) {
    SDL_Event event;
    int start_time = SDL_GetTicks();
    for (;;) {
        int ret;
        if (timeout_ms <= 0)
            ret = SDL_WaitEvent(&event);
        else
            ret = SDL_WaitEventTimeout(
                    &event, timeout_ms - (SDL_GetTicks() - start_time));
        if (ret == 0)
            return 0;
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

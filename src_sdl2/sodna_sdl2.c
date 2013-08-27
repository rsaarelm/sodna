#include "sodna.h"
#include <SDL.h>

static SDL_Window* g_win = NULL;
static SDL_Renderer* g_rend = NULL;

int sodna_init() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        return 1;

    g_win = SDL_CreateWindow(
            "", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            640, 400, SDL_WINDOW_SHOWN);
    if (!g_win)
        return 1;

    g_rend = SDL_CreateRenderer(g_win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    return 0;
}

void sodna_exit() {
    SDL_DestroyRenderer(g_rend); g_rend = NULL;
    SDL_DestroyWindow(g_win); g_win = NULL;
    SDL_Quit();
}

int sodna_map_color(unsigned char r, unsigned char g, unsigned char b) {
    // TODO
    return 0;
}

void sodna_set(int x, int y, int fore_col, int back_col, int symbol) {
    // TODO
}

void sodna_flush()
{
    // TODO
}

int sodna_wait_event()
{
    SDL_Event event;
    SDL_WaitEvent(&event);

    // TODO
    return 0;
}

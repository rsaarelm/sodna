/*
 * Sodna helper utilities. These aren't dependent on system library bindings
 * like Sodna core.
 */

#include "sodna.h"
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <stdint.h>
#include <stdlib.h>

int sodna_save_screenshot_png(const char* path) {
    int ret;
    int w, h;
    void* buffer = malloc(sodna_dump_screenshot(NULL, &w, &h));
    sodna_dump_screenshot(buffer, &w, &h);
    ret = stbi_write_png(path, w, h, 3, buffer, 0);
    free(buffer);
    return ret ? SODNA_ERROR : SODNA_OK;
}

int sodna_load_font(char* path, sodna_Font** out_font) {
    int w, h, n, font_w, font_h, i;
    uint8_t* data = stbi_load(path, &w, &h, &n, 1);
    if (!data) return SODNA_ERROR;

    *out_font = (sodna_Font*)malloc(sizeof(sodna_Font) + w * h);
    for (i = 0; i < w * h; i++)
        (*out_font)->pixel_data[i] = data[i];
    stbi_image_free(data);

    (*out_font)->char_width = w / 16;
    (*out_font)->char_height = h / 16;
    (*out_font)->pitch = w;

    return SODNA_OK;
}

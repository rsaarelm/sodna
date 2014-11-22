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
    void* buffer = malloc(sodna_dump_screenshot(NULL));
    sodna_dump_screenshot(buffer);
    ret = stbi_write_png(
            path,
            sodna_font_width() * sodna_width(),
            sodna_font_height() * sodna_height(),
            3, buffer, 0);
    free(buffer);
    return ret ? SODNA_ERROR : SODNA_OK;
}

int sodna_load_font_sheet(char* path) {
    int w, h, n, font_w, font_h;
    uint8_t* data = stbi_load(path, &w, &h, &n, 1);
    if (!data)
        return SODNA_ERROR;
    font_w = w / 16;
    font_h = h / 16;
    sodna_resize(font_w, font_h, sodna_width(), sodna_height());
    sodna_load_font_data(data, w, h, 0);
    stbi_image_free(data);
    return SODNA_OK;
}

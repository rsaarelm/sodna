#ifndef _SODNA_UTIL_H
#define _SODNA_UTIL_H

/**
 * Save screenshot to file as PNG image.
 */
int sodna_save_screenshot_png(const char* path);

/**
 * Load font data from a PNG, BMP or GIF image file. The font sheet must be
 * row-major table of 16x16 characters.
 */
int sodna_load_font_sheet(char* path);
#endif

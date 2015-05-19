#ifndef _SODNA_UTIL_H
#define _SODNA_UTIL_H

/** \file sodna_util.h */

#include "sodna.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Save screenshot to file as PNG image.
 */
int sodna_save_screenshot_png(const char* path);

/**
 * Load font data from a PNG, BMP or GIF image file.
 *
 * The font sheet must be row-major table of 16x16 characters.
 *
 * \param out_font Resulting font will be written here if load is
 * successful. The caller is responsible for freeing the data.
 *
 * \return \a SODNA_OK if successful, \a SODNA_ERROR otherwise.
 */
int sodna_load_font(char* path, sodna_Font** out_font);

#ifdef __cplusplus
}
#endif

#endif

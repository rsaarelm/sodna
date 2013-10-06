#ifndef _SODNA_H
#define _SODNA_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SODNA_VERSION "0.0.0"

enum Key {
    SODNA_CLOSE_WINDOW = -1,

    SODNA_UP = 1,
    SODNA_DOWN = 3,
    SODNA_LEFT = 4,
    SODNA_RIGHT = 2,
    SODNA_HOME = 5,
    SODNA_END = 6,
    SODNA_KP5 = 7,
    SODNA_BACKSPACE = 8,
    SODNA_TAB = 9,
    SODNA_ENTER = 10,
    SODNA_PAGEUP = 11,
    SODNA_PAGEDOWN = 12,
    SODNA_INSERT = 13,
    SODNA_DEL = 14,
    SODNA_F1 = 15,
    SODNA_F2 = 16,
    SODNA_F3 = 17,
    SODNA_F4 = 18,
    SODNA_F5 = 19,
    SODNA_F6 = 20,
    SODNA_F7 = 21,
    SODNA_F8 = 22,
    SODNA_F9 = 23,
    SODNA_F10 = 24,
    SODNA_F11 = 25,
    SODNA_F12 = 26,
    SODNA_ESC = 27,
};

/**
 * \brief Start the terminal.
 *
 * \return Zero on success, nonzero on error
 */
int sodna_init(
        int font_width,
        int font_height,
        int num_columns,
        int num_rows,
        const char* window_title);

/**
 * \brief Shut down the running terminal.
 */
void sodna_exit();

/**
 * \brief Get the width of a single character in pixels.
 */
int sodna_font_width();

/**
 * \brief Get the height of a single character in pixels.
 */
int sodna_font_height();

/**
 * \brief Load font pixel data from an 8-bit grayscale memory
 * source.
 *
 * Rows of characters are treated as contiguous in the pixel data.
 * You can specify the character code of the first character in the
 * pixel data to only load in a part of the font.
 */
void sodna_load_font_data(
        uint8_t* pixels,
        int pixels_width,
        int pixels_height,
        int first_char);

/**
 * \brief Get the width in columns of the terminal window.
 */
int sodna_width();

/**
 * \brief Get the height in rows of the terminal window.
 */
int sodna_height();

/**
 * \brief Terminal cell data structure.
 *
 * Each cell has an 8-bit ASCII symbol value, a 12-bit foreground color and a
 * 12-bit background color (each color channel can have a value from 0 to 15).
 * The entire structure fits in one 32-bit machine word, so it can be passed
 * around as value.
 */
typedef struct {
    unsigned int symbol: 8;
    unsigned int fore_b: 4;
    unsigned int fore_g: 4;
    unsigned int fore_r: 4;
    unsigned int back_b: 4;
    unsigned int back_g: 4;
    unsigned int back_r: 4;
} sodna_Cell;

/**
 * \brief Return the pointer to the screen memory of sodna_width() *
 * sodna_height() terminal cells.
 *
 * Write values to this memory to display things.
 */
sodna_Cell* sodna_cells();

/**
 * \brief Display the terminal with the changes.
 */
void sodna_flush();

/**
 * \brief Wait for an input event.
 *
 * \return Event code
 */
int sodna_wait_event();

/**
 * \brief Poll for input events.
 *
 * \return Event code of the first queued input or 0 if there are no pending
 * inputs.
 */
int sodna_poll_event();

#ifdef __cplusplus
}
#endif

#endif

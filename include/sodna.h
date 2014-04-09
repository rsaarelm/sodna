#ifndef _SODNA_H
#define _SODNA_H

/*
 * ##### ##### ###   ###   #####  A lightweight text game library
 * #     #   # #  #  #  #  #   #  Copyright (C) Risto Saarelma 2013-2014
 * ##### #   # #   # #   # #####
 *     # #   # #   # #   # #   #  MIT License
 * ##### ##### ##### #   # #   #
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SODNA_VERSION "0.2.0-pre"

/*
 * The event information is encoded into a signed 32-bit integer as a bit
 * pattern. The interpretation varies based on the type of the event.
 *
 * Event bit layouts
 *
 * Keyboard events:
 * KKKKKKKK KKKKKKKK SSSSSSSS XXXXXXX0
 *
 * K: Printable key in Unicode-16. Zero if a non-printable key was pressed or
 * if the event is key release.
 *
 * S: Keyboard layout independent scancode translated to sodna_Scancode. Signed
 * 8-bit integer. Positive if key was pressed, negative if it was released. The
 * absolute value of the scancode always corresponds to sodna_Scancode
 * constants.
 *
 * X: Reserved for future use.
 *
 * 0: Zero, only positive event integers correspond to key events.
 *
 * Other events:
 * EEEEEEEP PPPPPPPP PPPPPPPP PPPPPPP1
 *
 * E: 7-bit event identifier
 * P: 24-bit parameter space
 * 1: One, only negative integers correspond to non-key events.
 */

/**
 * Return the non-keypress event code for an event return value.
 *
 * Event codes are negative numbers. The highest bit is used to mark the value
 * as negative, and the low 7 bits are used to contain the event code. The
 * remaining bits can contain parameter data for the event.
 */
#define SODNA_EVENT(code) ((code < 0) ? (code | 0x7fffff80) : 0)
#define SODNA_EVENT_X(code) ((code >> 7) & 0xfff)
#define SODNA_EVENT_Y(code) ((code >> 19) & 0xfff)
#define SODNA_EVENT_MOUSE_BUTTON(code) ((code >> 7) & 0x3)

enum sodna_Event {
    SODNA_CLOSE_WINDOW = -1,
    SODNA_FOCUS_LOST = -2,
    SODNA_FOCUS_GAINED = -3,
    /* Bits 7 .. 19 contain the x and 20 .. 31 the y coordinate of the mouse. */
    SODNA_MOUSE_MOVED = -4,
    /* Bits 7 .. 9 contain the button id. */
    SODNA_MOUSE_DOWN = -5,
    /* Bits 7 .. 9 contain the button id. */
    SODNA_MOUSE_UP = -6,
    /* Bits 7 .. 8 contain delta, -1 (0x3), 0 (0x0), or 1 (0x1). */
    SODNA_MOUSE_WHEEL = -7,
};

#define SODNA_PRINTABLE_MASK 0x0000ffff
#define SODNA_SCANCODE(event) ((signed char)((event >> 16) & 0xff))

enum sodna_Scancode {
    SODNA_SCANCODE_UNKNOWN =        1,
    SODNA_SCANCODE_SPACE =          2,
    SODNA_SCANCODE_APOSTROPHE =     3,
    SODNA_SCANCODE_COMMA =          4,
    SODNA_SCANCODE_MINUS =          5,
    SODNA_SCANCODE_PERIOD =         6,
    SODNA_SCANCODE_SLASH =          7,
    SODNA_SCANCODE_0 =              8,
    SODNA_SCANCODE_1 =              9,
    SODNA_SCANCODE_2 =              10,
    SODNA_SCANCODE_3 =              11,
    SODNA_SCANCODE_4 =              12,
    SODNA_SCANCODE_5 =              13,
    SODNA_SCANCODE_6 =              14,
    SODNA_SCANCODE_7 =              15,
    SODNA_SCANCODE_8 =              16,
    SODNA_SCANCODE_9 =              17,
    SODNA_SCANCODE_SEMICOLON =      18,
    SODNA_SCANCODE_EQUALS =         19,
    SODNA_SCANCODE_A =              20,
    SODNA_SCANCODE_B =              21,
    SODNA_SCANCODE_C =              22,
    SODNA_SCANCODE_D =              23,
    SODNA_SCANCODE_E =              24,
    SODNA_SCANCODE_F =              25,
    SODNA_SCANCODE_G =              26,
    SODNA_SCANCODE_H =              27,
    SODNA_SCANCODE_I =              28,
    SODNA_SCANCODE_J =              29,
    SODNA_SCANCODE_K =              30,
    SODNA_SCANCODE_L =              31,
    SODNA_SCANCODE_M =              32,
    SODNA_SCANCODE_N =              33,
    SODNA_SCANCODE_O =              34,
    SODNA_SCANCODE_P =              35,
    SODNA_SCANCODE_Q =              36,
    SODNA_SCANCODE_R =              37,
    SODNA_SCANCODE_S =              38,
    SODNA_SCANCODE_T =              39,
    SODNA_SCANCODE_U =              40,
    SODNA_SCANCODE_V =              41,
    SODNA_SCANCODE_W =              42,
    SODNA_SCANCODE_X =              43,
    SODNA_SCANCODE_Y =              44,
    SODNA_SCANCODE_Z =              45,
    SODNA_SCANCODE_LEFTBRACKET =    46,
    SODNA_SCANCODE_BACKSLASH =      47,
    SODNA_SCANCODE_RIGHTBRACKET =   48,
    SODNA_SCANCODE_GRAVE =          49,
    SODNA_SCANCODE_ESCAPE =         50,
    SODNA_SCANCODE_ENTER =          51,
    SODNA_SCANCODE_TAB =            52,
    SODNA_SCANCODE_BACKSPACE =      53,
    SODNA_SCANCODE_INSERT =         54,
    SODNA_SCANCODE_DELETE =         55,
    SODNA_SCANCODE_RIGHT =          56,
    SODNA_SCANCODE_LEFT =           57,
    SODNA_SCANCODE_DOWN =           58,
    SODNA_SCANCODE_UP =             59,
    SODNA_SCANCODE_PAGE_UP =        60,
    SODNA_SCANCODE_PAGE_DOWN =      61,
    SODNA_SCANCODE_HOME =           62,
    SODNA_SCANCODE_END =            63,
    SODNA_SCANCODE_CAPS_LOCK =      64,
    SODNA_SCANCODE_SCROLL_LOCK =    65,
    SODNA_SCANCODE_NUM_LOCK =       66,
    SODNA_SCANCODE_PRINT_SCREEN =   67,
    SODNA_SCANCODE_PAUSE =          68,
    SODNA_SCANCODE_F1 =             69,
    SODNA_SCANCODE_F2 =             70,
    SODNA_SCANCODE_F3 =             71,
    SODNA_SCANCODE_F4 =             72,
    SODNA_SCANCODE_F5 =             73,
    SODNA_SCANCODE_F6 =             74,
    SODNA_SCANCODE_F7 =             75,
    SODNA_SCANCODE_F8 =             76,
    SODNA_SCANCODE_F9 =             77,
    SODNA_SCANCODE_F10 =            78,
    SODNA_SCANCODE_F11 =            79,
    SODNA_SCANCODE_F12 =            80,
    SODNA_SCANCODE_KP_0 =           81,
    SODNA_SCANCODE_KP_1 =           82,
    SODNA_SCANCODE_KP_2 =           83,
    SODNA_SCANCODE_KP_3 =           84,
    SODNA_SCANCODE_KP_4 =           85,
    SODNA_SCANCODE_KP_5 =           86,
    SODNA_SCANCODE_KP_6 =           87,
    SODNA_SCANCODE_KP_7 =           88,
    SODNA_SCANCODE_KP_8 =           89,
    SODNA_SCANCODE_KP_9 =           90,
    SODNA_SCANCODE_KP_DECIMAL =     91,
    SODNA_SCANCODE_KP_DIVIDE =      92,
    SODNA_SCANCODE_KP_MULTIPLY =    93,
    SODNA_SCANCODE_KP_MINUS =       94,
    SODNA_SCANCODE_KP_PLUS =        95,
    SODNA_SCANCODE_KP_ENTER =       96,
    SODNA_SCANCODE_KP_EQUALS =      97,
    SODNA_SCANCODE_LEFT_SHIFT =     98,
    SODNA_SCANCODE_LEFT_CONTROL =   99,
    SODNA_SCANCODE_LEFT_ALT =       100,
    SODNA_SCANCODE_LEFT_SUPER =     101,
    SODNA_SCANCODE_RIGHT_SHIFT =    102,
    SODNA_SCANCODE_RIGHT_CONTROL =  103,
    SODNA_SCANCODE_RIGHT_ALT =      104,
    SODNA_SCANCODE_RIGHT_SUPER =    105,
};


enum sodna_Button {
    SODNA_LEFT_BUTTON = 0,
    SODNA_MIDDLE_BUTTON = 1,
    SODNA_RIGHT_BUTTON = 2,
};

typedef enum {
    SODNA_OK = 0,
    SODNA_ERROR = 1,
    SODNA_UNSUPPORTED = 2,
} sodna_Error;

/**
 * \brief Start the terminal.
 *
 * \return Zero on success, nonzero on error
 */
sodna_Error sodna_init(
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
        const uint8_t* pixels,
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
 * \brief Set the color of the edges around the cells. Color format
 * is 0xRGB.
 */
void sodna_set_edge_color(int color);

/**
 * \brief Display the terminal with the changes.
 */
void sodna_flush();

/**
 * \brief Wait for an input event.
 *
 * \param timeout_ms If positive, indicate that the function should not wait
 * beyond that number of milliseconds. If 0, indicate that the function should
 * wait until an event arrives.
 *
 * \return Event code or 0 if the wait timed out.
 */
int sodna_wait_event(int timeout_ms);

/**
 * \brief Poll for input events.
 *
 * \return Event code of the first queued input or 0 if there are no pending
 * inputs.
 */
int sodna_poll_event();

/**
 * \brief Return time in milliseconds since Sodna init.
 *
 * \return -1 if timing is not supported, otherwise the number of milliseconds
 * elapsed.
 */
int sodna_ms_elapsed();

/**
 * \brief Suspend program for given number of milliseconds.
 */
sodna_Error sodna_sleep_ms(int ms);

/**
 * \brief Save a screenshot of the current screen to disk.
 */
sodna_Error sodna_save_screenshot(const char* path);

#ifdef __cplusplus
}
#endif

#endif

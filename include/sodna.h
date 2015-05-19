#ifndef _SODNA_H
#define _SODNA_H

/** \file sodna.h */

/*
 * ##### ##### ###   ###   #####  A lightweight text game library
 * #     #   # #  #  #  #  #   #  Copyright (C) Risto Saarelma 2013-2015
 * ##### #   # #   # #   # #####
 *     # #   # #   # #   # #   #  MIT License
 * ##### ##### ##### #   # #   #
 */

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SODNA_VERSION "0.3.0-pre"

/**
 * Status codes for operations that may fail
 */
typedef enum {
    SODNA_OK = 0,
    SODNA_ERROR = 1,
    SODNA_UNSUPPORTED = 2,
} sodna_Error;

/**
 * Start the Sodna terminal.
 *
 * Start using Sodna by calling \a sodna_init. Specify the pixel font
 * dimensions to determine the shape of the characters and the size of
 * the font pixel sheet. Specify column and row count to determine how
 * much text will fit on the window.
 *
 * \return SODNA_OK or SODNA_ERROR if opening the terminal failed.
 */
sodna_Error sodna_init(
        int font_width,
        int font_height,
        int num_columns,
        int num_rows,
        const char* window_title);

/**
 * Color data structure
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} sodna_Color;

/**
 * Terminal cell data structure
 *
 * Each cell has a foreground and background color value and an ASCII
 * symbol value. The whole cell is padded to 64 bits.
 */
typedef struct {
    sodna_Color fore;
    sodna_Color back;
    unsigned int symbol: 8;
    unsigned int reserved: 8;
} sodna_Cell;

/**
 * Return the pointer to the screen memory of sodna_width() *
 * sodna_height() terminal cells.
 *
 * Write values to this memory to display things.
 */
sodna_Cell* sodna_cells();

/**
 * Display the terminal with the changes.
 */
void sodna_flush();

/**
 * Shut down the running terminal.
 */
void sodna_exit();

/**
 * Get the width of a single character in pixels.
 */
int sodna_font_width();

/**
 * Get the height of a single character in pixels.
 */
int sodna_font_height();

/**
 * Get the width in columns of the terminal window.
 */
int sodna_width();

/**
 * Get the height in rows of the terminal window.
 */
int sodna_height();

/**
 * Set the color of the edges around the cells.
 */
void sodna_set_edge_color(sodna_Color color);

/**
 * Toggle fullscreen mode, 1 for fullscreen, 0 for windowed
 *
 * \return error code if not supported by backend
 */
sodna_Error sodna_set_fullscreen(int is_fullscreen_mode);

/*
 * The events are returned as sodna_Event unions. The type field has one
 * of the SODNA_EVENT* values and tells which type of actual event
 * struct the sodna_Event should be interpreted as.
 */

/** Event for a key press or key release */
typedef struct {
    uint8_t type;
    /** Pressed key according to the user's keyboard layout */
    unsigned layout: 8;
    /** Layout independent hardware key, interpreted as if keyboard was US QWERTY */
    unsigned hardware: 8;
    // Modifier keys
    unsigned shift: 1;
    unsigned ctrl: 1;
    unsigned alt: 1;
    unsigned super: 1;
    unsigned caps_lock: 1;
} sodna_KeyPressed;

/** Event for a printable character typed */
typedef struct {
    uint8_t type;
    /** Unicode codepoint for the character */
    unsigned code: 24;
} sodna_CharTyped;

/** Event for a mouse move */
typedef struct {
    uint8_t type;
    unsigned x: 12;
    unsigned y: 12;
} sodna_MouseMove;

/** Event for a mouse button press or release */
typedef struct {
    uint8_t type;
    /** Bit field of SODNA_*_BUTTON values */
    unsigned id: 8;
} sodna_MouseButton;

#define SODNA_LEFT_BUTTON 0
#define SODNA_MIDDLE_BUTTON 1
#define SODNA_RIGHT_BUTTON 2

/** Mouse wheel event */
typedef struct {
    uint8_t type;
    signed delta: 8;
} sodna_MouseWheel;

/** Event union */
typedef union {
    uint8_t type;
    sodna_KeyPressed key;
    sodna_CharTyped ch;
    sodna_MouseMove mouse;
    sodna_MouseButton button;
    sodna_MouseWheel wheel;
} sodna_Event;

/*
 * Identifier codes for Sodna events.
 */

#define SODNA_EVENT_NONE            0x00

/* No parameters */
#define SODNA_EVENT_CLOSE_WINDOW    0x01
#define SODNA_EVENT_FOCUS_GAINED    0x02
#define SODNA_EVENT_FOCUS_LOST      0x82

/* sodna_KeyPressed parameters */
#define SODNA_EVENT_KEY_DOWN        0x03
#define SODNA_EVENT_KEY_UP          0x83

/* sodna_CharTyped parameters */
#define SODNA_EVENT_CHARACTER       0x04

/* sodna_MouseMove parameters */
#define SODNA_EVENT_MOUSE_MOVED     0x05

/* sodna_MouseButton parameters */
#define SODNA_EVENT_MOUSE_DOWN      0x06
#define SODNA_EVENT_MOUSE_UP        0x86

/* sodna_MouseWheel parameters */
#define SODNA_EVENT_MOUSE_WHEEL     0x07

/* No parameters */
#define SODNA_EVENT_MOUSE_ENTER     0x08
#define SODNA_EVENT_MOUSE_EXIT      0x88

/**
 * Resize the font and window during runtime.
 */
void sodna_resize(int font_width, int font_height, int num_columns, int num_rows);

/**
 * Load font pixel data from an 8-bit grayscale memory source.
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
 * Wait for an input event.
 *
 * \param timeout_ms If positive, indicate that the function should not wait
 * beyond that number of milliseconds. If 0, indicate that the function should
 * wait until an event arrives.
 *
 * \return Event code or 0 if the wait timed out.
 */
sodna_Event sodna_wait_event(int timeout_ms);

/**
 * Poll for input events.
 *
 * \return Event code of the first queued input or 0 if there are no pending
 * inputs.
 */
sodna_Event sodna_poll_event();

/**
 * Return time in milliseconds since Sodna init.
 *
 * \return -1 if timing is not supported, otherwise the number of milliseconds
 * elapsed.
 */
int sodna_ms_elapsed();

/**
 * Suspend program for given number of milliseconds.
 */
sodna_Error sodna_sleep_ms(int ms);

/**
 * Write the RGB8 pixels of the current screenshot to user-provided
 * memory.
 *
 * Returns the number of bytes written. If called with a NULL value will not
 * write anything but will return the size of the screen dump anyway. The user
 * can use this to find out how much space to allocate.
 *
 * May return 0 if the backend implementation does not support screenshots.
 */
size_t sodna_dump_screenshot(void* dest);

/* Keyboard keys */
#define SODNA_KEY_UNKNOWN          1
#define SODNA_KEY_SPACE            2
#define SODNA_KEY_APOSTROPHE       3
#define SODNA_KEY_COMMA            4
#define SODNA_KEY_MINUS            5
#define SODNA_KEY_PERIOD           6
#define SODNA_KEY_SLASH            7
#define SODNA_KEY_0                8
#define SODNA_KEY_1                9
#define SODNA_KEY_2                10
#define SODNA_KEY_3                11
#define SODNA_KEY_4                12
#define SODNA_KEY_5                13
#define SODNA_KEY_6                14
#define SODNA_KEY_7                15
#define SODNA_KEY_8                16
#define SODNA_KEY_9                17
#define SODNA_KEY_SEMICOLON        18
#define SODNA_KEY_EQUALS           19
#define SODNA_KEY_A                20
#define SODNA_KEY_B                21
#define SODNA_KEY_C                22
#define SODNA_KEY_D                23
#define SODNA_KEY_E                24
#define SODNA_KEY_F                25
#define SODNA_KEY_G                26
#define SODNA_KEY_H                27
#define SODNA_KEY_I                28
#define SODNA_KEY_J                29
#define SODNA_KEY_K                30
#define SODNA_KEY_L                31
#define SODNA_KEY_M                32
#define SODNA_KEY_N                33
#define SODNA_KEY_O                34
#define SODNA_KEY_P                35
#define SODNA_KEY_Q                36
#define SODNA_KEY_R                37
#define SODNA_KEY_S                38
#define SODNA_KEY_T                39
#define SODNA_KEY_U                40
#define SODNA_KEY_V                41
#define SODNA_KEY_W                42
#define SODNA_KEY_X                43
#define SODNA_KEY_Y                44
#define SODNA_KEY_Z                45
#define SODNA_KEY_LEFTBRACKET      46
#define SODNA_KEY_BACKSLASH        47
#define SODNA_KEY_RIGHTBRACKET     48
#define SODNA_KEY_GRAVE            49
#define SODNA_KEY_ESCAPE           50
#define SODNA_KEY_ENTER            51
#define SODNA_KEY_TAB              52
#define SODNA_KEY_BACKSPACE        53
#define SODNA_KEY_INSERT           54
#define SODNA_KEY_DELETE           55
#define SODNA_KEY_RIGHT            56
#define SODNA_KEY_LEFT             57
#define SODNA_KEY_DOWN             58
#define SODNA_KEY_UP               59
#define SODNA_KEY_PAGE_UP          60
#define SODNA_KEY_PAGE_DOWN        61
#define SODNA_KEY_HOME             62
#define SODNA_KEY_END              63
#define SODNA_KEY_CAPS_LOCK        64
#define SODNA_KEY_SCROLL_LOCK      65
#define SODNA_KEY_NUM_LOCK         66
#define SODNA_KEY_PRINT_SCREEN     67
#define SODNA_KEY_PAUSE            68
#define SODNA_KEY_F1               69
#define SODNA_KEY_F2               70
#define SODNA_KEY_F3               71
#define SODNA_KEY_F4               72
#define SODNA_KEY_F5               73
#define SODNA_KEY_F6               74
#define SODNA_KEY_F7               75
#define SODNA_KEY_F8               76
#define SODNA_KEY_F9               77
#define SODNA_KEY_F10              78
#define SODNA_KEY_F11              79
#define SODNA_KEY_F12              80
#define SODNA_KEY_KP_0             81
#define SODNA_KEY_KP_1             82
#define SODNA_KEY_KP_2             83
#define SODNA_KEY_KP_3             84
#define SODNA_KEY_KP_4             85
#define SODNA_KEY_KP_5             86
#define SODNA_KEY_KP_6             87
#define SODNA_KEY_KP_7             88
#define SODNA_KEY_KP_8             89
#define SODNA_KEY_KP_9             90
#define SODNA_KEY_KP_DECIMAL       91
#define SODNA_KEY_KP_DIVIDE        92
#define SODNA_KEY_KP_MULTIPLY      93
#define SODNA_KEY_KP_MINUS         94
#define SODNA_KEY_KP_PLUS          95
#define SODNA_KEY_KP_ENTER         96
#define SODNA_KEY_KP_EQUALS        97
#define SODNA_KEY_LEFT_SHIFT       98
#define SODNA_KEY_LEFT_CONTROL     99
#define SODNA_KEY_LEFT_ALT         100
#define SODNA_KEY_LEFT_SUPER       101
#define SODNA_KEY_RIGHT_SHIFT      102
#define SODNA_KEY_RIGHT_CONTROL    103
#define SODNA_KEY_RIGHT_ALT        104
#define SODNA_KEY_RIGHT_SUPER      105

#ifdef __cplusplus
}
#endif

#endif

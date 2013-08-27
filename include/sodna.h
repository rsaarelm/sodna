#ifndef _SODNA_H
#define _SODNA_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Start the terminal.
 *
 * \return Zero on success, nonzero on error
 */
int sodna_init();

/**
 * \brief Shut down the running terminal.
 */
void sodna_exit();

/**
 * \brief Get the width in columns of the terminal window.
 */
int sodna_width();

/**
 * \brief Get the height in rows of the terminal window.
 */
int sodna_height();

/**
 * \brief Convert a color value into the terminal's internal
 * representation.
 */
int sodna_map_color(unsigned char r, unsigned char g, unsigned char b);

/**
 * \brief Set the symbol and color in a given terminal cell.
 *
 * The color values must be in the terminal's internal
 * representation given by a sodna_map_color call.
 */
void sodna_set(int x, int y, int fore_color, int back_color, int symbol);

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

#ifndef _SODNA_H
#define _SODNA_H

#include <stdint.h>

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

typedef struct {
    unsigned int symbol: 8;
    unsigned int fore_r: 4;
    unsigned int fore_g: 4;
    unsigned int fore_b: 4;
    unsigned int back_r: 4;
    unsigned int back_g: 4;
    unsigned int back_b: 4;
} sodna_Cell;

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

/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

#ifndef CONF_BOARD_H
#define CONF_BOARD_H


// Image sensor VSYNC pin.
#define OV7740_VSYNC_PIO		PIOA
#define OV7740_VSYNC_ID			ID_PIOA
#define OV7740_VSYNC_MASK		PIO_PA15
#define OV7740_VSYNC_TYPE		PIO_PULLUP

#define SRAM_BASE                      (0x60000000UL) // SRAM adress
#define CAP_DEST                       SRAM_BASE

#define IMAGE_WIDTH                    (320UL)
#define IMAGE_HEIGHT                   (240UL)

#endif // CONF_BOARD_H

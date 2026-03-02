/**
 * \file
 *
 * \brief User board configuration template
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

// Put universal pin declarations in this file

#ifndef CONF_BOARD_H
#define CONF_BOARD_H

#define BOARD_FREQ_SLCK_XTAL (32768U)
#define BOARD_FREQ_SLCK_BYPASS (32768U)
#define BOARD_FREQ_MAINCK_XTAL (12000000U)
#define BOARD_FREQ_MAINCK_BYPASS (12000000U)
#define BOARD_MCK				CHIP_FREQ_CPU_MAX
#define BOARD_OSC_STARTUP_US	15625

#define EXAMPLE_LED PIO_PA19_IDX
#define BUTTON_PIN PIO_PA23_IDX

// Image sensor VSYNC pin
#define OV7740_VSYNC_PIO		PIOA
#define OV7740_VSYNC_ID			ID_PIOA
#define OV7740_VSYNC_MASK		PIO_PA15
#define OV7740_VSYNC_TYPE		PIO_PULLUP

#define SRAM_BASE                      (0x60000000UL) // SRAM address
#define CAP_DEST                       SRAM_BASE

#define IMAGE_WIDTH                    (320UL)
#define IMAGE_HEIGHT                   (240UL)

// TWI Interface
#define ID_BOARD_TWI		               ID_TWI0
#define BOARD_TWI			                 TWI0
#define BOARD_TWI_IRQn		             TWI0_IRQn

// Camera PCK0 (XCLK) Pin
#define PIN_PCK0                       (PIO_PA17_IDX)
#define PIN_PCK0_FLAGS                 (PIO_PERIPH_A | PIO_DEFAULT)

// Camera Data Bus Pins
#define OV_DATA_BUS_D0                 PIO_PA24_IDX
#define OV_DATA_BUS_D1                 PIO_PA25_IDX
#define OV_DATA_BUS_D2                 PIO_PA26_IDX
#define OV_DATA_BUS_D3                 PIO_PA27_IDX
#define OV_DATA_BUS_D4                 PIO_PA28_IDX
#define OV_DATA_BUS_D5                 PIO_PA29_IDX
#define OV_DATA_BUS_D6                 PIO_PA30_IDX
#define OV_DATA_BUS_D7                 PIO_PA31_IDX
#define OV_DATA_BUS_FLAGS              (PIO_INPUT | PIO_PULLUP)
#define OV_DATA_BUS_MASK               (0xFF000000UL)
#define OV_DATA_BUS_PIO                PIOA
#define OV_DATA_BUS_ID                 ID_PIOA
#define OV_DATA_BUS_TYPE               PIO_INPUT
#define OV_DATA_BUS_ATTR               PIO_DEFAULT

// HSYNC and VSYNC Signals
#define OV_HSYNC_GPIO                  PIO_PA16_IDX
#define OV_HSYNC_FLAGS                 (PIO_PULLUP | PIO_IT_RISE_EDGE)

#define OV_VSYNC_GPIO                  PIO_PA15_IDX
#define OV_VSYNC_FLAGS                 (PIO_PULLUP | PIO_IT_RISE_EDGE)

// PCLK
#define OV_PCLK_GPIO					PIO_PA23_IDX
#define OV_PCLK_FLAGS					(PIO_INPUT)

// SDA and SCL
#define OV_SDA_GPIO						PIO_PA3_IDX
#define OV_SCL_GPIO						PIO_PA4_IDX

#endif // CONF_BOARD_H
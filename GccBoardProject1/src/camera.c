/*
 * camera.c
 *
 * Created: 2/23/2026 10:25:20 AM
 *  Author: laptop
 */ 

#include "camera.h"
#include "ov2640.h"
#include "timer_interface.h"

#define TWI_CLK     (400000UL)

/* Pointer to the image data destination buffer */
uint8_t *g_p_uc_cap_dest_buf;

/* Rows and line size of capturing picture */
uint16_t g_us_cap_rows = IMAGE_HEIGHT;
uint16_t g_us_cap_line = IMAGE_WIDTH * 2;

/* Vsync signal information (true if it's triggered and false otherwise) */
static volatile uint32_t g_ul_vsync_flag = false;

// Image length
volatile uint32_t image_len = 0;

static void vsync_handler(uint32_t ul_id, uint32_t ul_mask)
{
	unused(ul_id);
	unused(ul_mask);

	g_ul_vsync_flag = true;
}

void init_vsync_interrupts(void)
{
	/* Initialize PIO interrupt handler, see PIO definition in conf_board.h
	**/
	pio_handler_set(OV7740_VSYNC_PIO, OV7740_VSYNC_ID, OV7740_VSYNC_MASK,
			OV7740_VSYNC_TYPE, vsync_handler);

	/* Enable PIO controller IRQs */
	NVIC_EnableIRQ((IRQn_Type)OV7740_VSYNC_ID);
}

static void configure_twi(void) {
	twi_options_t opt;

	/* Enable TWI peripheral */
	pmc_enable_periph_clk(ID_BOARD_TWI);

	/* Init TWI peripheral */
	opt.master_clk = sysclk_get_cpu_hz();
	opt.speed      = TWI_CLK;
	twi_master_init(BOARD_TWI, &opt);

	/* Configure TWI interrupts */
	NVIC_DisableIRQ(BOARD_TWI_IRQn);
	NVIC_ClearPendingIRQ(BOARD_TWI_IRQn);
	NVIC_SetPriority(BOARD_TWI_IRQn, 0);
	NVIC_EnableIRQ(BOARD_TWI_IRQn);

}

void pio_capture_init(Pio *p_pio, uint32_t ul_id) {
	/* Enable peripheral clock */
	pmc_enable_periph_clk(ul_id);

	/* Disable pio capture */
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_PCEN);

	/* Disable rxbuff interrupt */
	p_pio->PIO_PCIDR |= PIO_PCIDR_RXBUFF;

	/* 32bit width*/
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_DSIZE_Msk);
	p_pio->PIO_PCMR |= PIO_PCMR_DSIZE_WORD;

	/* Only HSYNC and VSYNC enabled */
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_ALWYS);
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_HALFS);

	#if !defined(DEFAULT_MODE_COLORED)
	/* Samples only data with even index */
	p_pio->PIO_PCMR |= PIO_PCMR_HALFS;
	p_pio->PIO_PCMR &= ~((uint32_t)PIO_PCMR_FRSTS);
	#endif
}



static uint8_t pio_capture_to_buffer(Pio *p_pio, uint8_t *uc_buf,
uint32_t ul_size)
{
	/* Check if the first PDC bank is free */
	if ((p_pio->PIO_RCR == 0) && (p_pio->PIO_RNCR == 0)) {
		p_pio->PIO_RPR = (uint32_t)uc_buf;
		p_pio->PIO_RCR = ul_size;
		p_pio->PIO_PTCR = PIO_PTCR_RXTEN;
		return 1;
		} else if (p_pio->PIO_RNCR == 0) {
		p_pio->PIO_RNPR = (uint32_t)uc_buf;
		p_pio->PIO_RNCR = ul_size;
		return 1;
		} else {
		return 0;
	}
}

void init_camera(void) {
	// INPUTS
	// D0 - PA24
	// D1 - PA25
	// D2 - PA26
	// D3 - PA27
	// D4 - PA28
	// D5 - PA29
	// D6 - PA30
	// D7 - PA31
	gpio_configure_pin(OV_DATA_BUS_D0, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D1, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D2, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D3, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D4, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D5, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D6, OV_DATA_BUS_FLAGS);
	gpio_configure_pin(OV_DATA_BUS_D7, OV_DATA_BUS_FLAGS);
		
	// PCLK - PA23 (synchronization clock)
	gpio_configure_pin(OV_PCLK_GPIO, OV_PCLK_FLAGS);
		
	// HSYNC - PA16	
	// VSYNC - PA15
	gpio_configure_pin(OV_HSYNC_GPIO, OV_HSYNC_FLAGS);
	gpio_configure_pin(OV_VSYNC_GPIO, OV_VSYNC_FLAGS);
	// OUTPUTS
	// XCLK - PA17
	gpio_configure_pin(PIN_PCK0, PIN_PCK0_FLAGS);
	
	// SDA
	// SCL
	gpio_configure_pin(OV_SCL_GPIO, PIO_PERIPH_A);
	gpio_configure_pin(OV_SDA_GPIO, PIO_PERIPH_A);
	
	pmc_enable_pck(0);
	while (!(PMC->PMC_SR & PMC_SR_PCKRDY0)) {}
	
	configure_twi();
	
	init_vsync_interrupts();
	
}

void configure_camera() {
	
	/* ov Initialization */
	while (ov_init(BOARD_TWI) == 1) {
	}

	/* ov7740 configuration */
	ov_configure(BOARD_TWI, JPEG_INIT);
	ov_configure(BOARD_TWI, YUV422);
	ov_configure(BOARD_TWI, JPEG);
	ov_configure(BOARD_TWI, JPEG_320x240);


	/* Wait 3 seconds to let the image sensor to adapt to environment */
	delay_ms(3000);
}

static uint8_t find_image_len(void) {
	uint8_t *buffer = g_p_uc_cap_dest_buf;

	uint32_t max = g_us_cap_rows * g_us_cap_line;
	
	uint32_t SOI_index = 0;
	uint32_t EOI_index = 0;
	uint32_t soi_found = 0;
	for (uint32_t i = 0; i < max-1; i++) {
		
		if (!soi_found && buffer[i] == 0xFF && buffer[i+1] == 0xD8) {
			SOI_index = i;
			soi_found += 1;
		}
		
		if (soi_found && buffer[i] == 0xFF && buffer[i+1] == 0xD9) {
			EOI_index = i;
			image_len = EOI_index - SOI_index + 2;
			return 1;
		}
	}
	
	return 0;	
}

uint8_t start_capture(void)
{
	/* Set capturing destination address*/
	g_p_uc_cap_dest_buf = (uint8_t *)CAP_DEST;

	/* Set cap_rows value*/
	g_us_cap_rows = IMAGE_HEIGHT;
	
	g_ul_vsync_flag = false;
	
	/* Enable vsync interrupt*/
	pio_enable_interrupt(OV7740_VSYNC_PIO, OV7740_VSYNC_MASK);

	/* Capture acquisition will start on rising edge of Vsync signal.
	 * So wait g_vsync_flag = 1 before start process
	 */
	
	uint32_t timeout = 10000000;
	 	
	while (!g_ul_vsync_flag && timeout--) {
	}

	/* Disable vsync interrupt*/
	pio_disable_interrupt(OV7740_VSYNC_PIO, OV7740_VSYNC_MASK);
	
	if (!g_ul_vsync_flag) {return 0;} // check for if we timed out
	
	/* Enable pio capture*/
	pio_capture_enable(OV_DATA_BUS_PIO);

	/* Capture data and send it to external SRAM memory thanks to PDC
	 * feature */
	uint32_t buffered = pio_capture_to_buffer(OV_DATA_BUS_PIO, g_p_uc_cap_dest_buf,
			(g_us_cap_line * g_us_cap_rows) >> 2);

	if (buffered == 0) {return 0;}
		

	/* Wait end of capture*/
	while (!((OV_DATA_BUS_PIO->PIO_PCISR & PIO_PCIMR_RXBUFF) ==
			PIO_PCIMR_RXBUFF)) {
	}

	/* Disable pio capture*/
	pio_capture_disable(OV_DATA_BUS_PIO);
	
	if (!find_image_len()) {return 0;}
	
	return 1;
}


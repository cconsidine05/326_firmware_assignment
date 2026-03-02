/*
 * wifi.c
 *
 * Created: 2/27/2026 12:29:37 PM
 *  Author: ccons
 */ 


#include "wifi.h"
#include <stdio.h>
#include <asf.h>


volatile char input_line_wifi[MAX_INPUT_WIFI];
volatile uint32_t received_byte_wifi = 0;
volatile bool new_rx_wifi = false;
volatile unsigned int input_pos_wifi = 0;
volatile bool wifi_comm_success = false;

volatile uint32_t transfer_len = 0;
volatile uint32_t transfer_index = 0;
volatile bool wifi_provision_flag = false;

// handler for incoming data from the WiFi.
// should call process incoming_byte when it arrives
// there is a pre declared handler because this is a peripheral. We have to declare it
void WIFI_USART_HANDLER(void)
{
	uint32_t ul_status;
	
	ul_status = usart_get_status(WIFI_USART);
	
	if(ul_status & US_CSR_RXBUFF){
		usart_read(WIFI_USART, &received_byte_wifi);
		new_rx_wifi = true;
		process_incoming_byte_wifi((uint8_t)received_byte_wifi);
	}

}
// stores every incoming byte from the ESP32 in a buffer

// That's it!
void process_incoming_byte_wifi(uint8_t byte){
	
	// should maybe validate that we don't overflow the buffer
	//ADD IF STATEMENT TO MAKE SURE INPUT_LINE_WIFI is less than 1000 bytes
	// Otherwise it will interfere with existing memory
	assert(input_pos_wifi < 1000);
	input_line_wifi[input_pos_wifi] = byte;
	input_pos_wifi++;
	
}

// handler for command complete rising edge interrupt from ESP 32. When it is triggered
// it is time to process the result from the esp32

void wifi_command_response_handler(uint32_t ul_id, uint32_t ul_mask){
	unused(ul_id);
	unused(ul_mask);
	
	wifi_comm_success = true;
	
	// process the entire buffer and reset it
	process_data_wifi();
	for(int i = 0; i < MAX_INPUT_WIFI; i++) input_line_wifi[i] = 0;
	input_pos_wifi = 0;
}

//// processes the reponse of the ESP32, which should be stored in the bugger filled
// by process incoming byte wifi. this processing should be looking for cetain responses
// that the ESP32 should give, such as "SUCCESS" when "test" is sent to it
// toggle an LED when we get the word success

// that's also it
void process_data_wifi(void){
	
	if (strstr(input_line_wifi, "SUCCESS")){
		ioport_toggle_pin_level((EXAMPLE_LED));
	}
}

// handler for button to initiate procisioning mode of the ESP32
// should set a flag indicating a request to initiate provisinoing mode


// look at fixing wifi_provision handler right now
void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask){
	
	unused(ul_id);
	unused(ul_mask);
	wifi_provision_flag = true;
	
	
}

// handler for peripheral mode interrupts on SPI bus. When the ESP32 SPi
// controller requests data, this interrupt should send one byte of the image
// at a time

// at the start the spi is 99
// the first time the interrupt is not ready and sends the last thing that was loaded in 
// from the previous transaction, which was 99
void wifi_spi_handler(void){
	
	uint32_t status;
	uint32_t dummy_data;
	uint8_t uc_pcs;
	
	// Read status to see why we are here
	status = spi_read_status(SPI);
	
	// Check if data was received (meaning the ESP32 sent a clock pulse)
	if(status & SPI_SR_RDRF){
		
		// CRITICAL: Read the data register to clear the interrupt flag
		// If you skip this, the interrupt will fire infinitely and freeze the MCU.
		spi_read(SPI, &dummy_data, &uc_pcs);
		
		// If we still have bytes left to send...
		if (transfer_len > 0){
			// Write the actual pixel data from your buffer
			spi_write(SPI, image_buffer[transfer_index], 0, 0);
			
			// Move to the next pixel
			transfer_index++;
			transfer_len--;
		}
		else {
			// No more data? Send 0 just to keep the line quiet.
			spi_write(SPI, 0, 0, 0);
		}
	}
}

// configuration of USART port used to communicate with the ESP32
// got this directlty from a usart demo project
void configure_usart_wifi(void){
	
	gpio_configure_pin(PIN_USART0_RXD_IDX, PIN_USART0_RXD_FLAGS);
	gpio_configure_pin(PIN_USART0_TXD_IDX, PIN_USART0_TXD_FLAGS);
	
	
	const sam_usart_opt_t usart_console_settings = {
		WIFI_USART_BAUDRATE,
		US_MR_CHRL_8_BIT,
		US_MR_PAR_NO,
		US_MR_NBSTOP_1_BIT,
		US_MR_CHMODE_NORMAL,
		// this next field is only used in IrDA mode
		0
	};
	
	// enable peripheral clock
	sysclk_enable_peripheral_clock(WIFI_USART_ID);
	
	usart_init_rs232(WIFI_USART, &usart_console_settings, sysclk_get_cpu_hz());
	
	usart_disable_interrupt(WIFI_USART, ALL_INTERRUPT_MASK);
	
	usart_enable_tx(WIFI_USART);
	usart_enable_rx(WIFI_USART);
	
	NVIC_EnableIRQ(WIFI_USART_IRQn);
	
	usart_enable_interrupt(WIFI_USART, US_IER_RXRDY);
	
}

// configuraiton of the "command complete" rising edge interrupt

void configure_wifi_comm_pin(void){
	
	pmc_enable_periph_clk(WIFI_COMM_ID);
	
	pio_handler_set(WIFI_COMM_PIO, WIFI_COMM_ID, WIFI_COMM_PIN_NUM, WIFI_COMM_ATTR, wifi_command_response_handler);
	NVIC_EnableIRQ((IRQn_Type) WIFI_COMM_ID);
	
	pio_enable_interrupt(WIFI_COMM_PIO, WIFI_COMM_PIN_NUM);
}

//configuraiton of button interrupt to initiate provisioning mode

void configure_wifi_provision_pin(void){
	
	pmc_enable_periph_clk(WIFI_PROV_PIN_ID);
	
	pio_set_input(WIFI_PROV_PIO, WIFI_PROV_MASK, PIO_PULLUP);
	
	pio_set_debounce_filter(WIFI_PROV_PIO, WIFI_PROV_MASK, 10);
	
	pio_handler_set(WIFI_PROV_PIO, WIFI_PROV_PIN_ID, WIFI_PROV_MASK, WIFI_PROV_ATTR, wifi_provision_handler);
	
	NVIC_EnableIRQ((IRQn_Type) WIFI_PROV_PIN_ID);
	
	pio_enable_interrupt(WIFI_PROV_PIO, WIFI_PROV_MASK);
	
}
// configuration of SPI port and interrupts used to send images to the ESP32

void configure_spi(void){
	gpio_configure_pin(SPI_MISO_GPIO, SPIO_MISO_FLAGS);
	gpio_configure_pin(SPI_MOSI_GPIO, SPI_MOSI_FLAGS);
	gpio_configure_pin(SPI_SPK_GPIO, SPI_SPCK_FLAGS);
	gpio_configure_pin(SPI_NPCS0_GPIO, SPI_NPCS0_FLAGS);
	
}

// initialize teh SPI port as a peripheral (slave) device. Note:
// the embedded instury is trying to phase out the "master/slave"

// the microcontroller is the master for this spi peripheral, not the master
void spi_peripheral_initialize(void){
	
	spi_enable_clock(SPI);
	spi_disable(SPI);
	spi_reset(SPI);
	spi_set_slave_mode(SPI);
	spi_disable_mode_fault_detect(SPI);
	spi_set_peripheral_chip_select_value(SPI, SPI_CHIP_PCS);
	spi_set_clock_polarity(SPI, SPI_CHIP_SEL, SPI_CLK_POLARITY);
	spi_set_clock_phase(SPI, SPI_CHIP_SEL, SPI_CLK_PHASE);
	spi_set_bits_per_transfer(SPI, SPI_CHIP_SEL, SPI_CSR_BITS_8_BIT);
	spi_enable_interrupt(SPI, SPI_IER_RDRF);
	spi_enable(SPI);
	
	
	
	
}

// set necessary parameters to perpare for SPI transfer. 

void prepare_spi_transfer(void){
	transfer_len = image_len;
	transfer_index = 0;
	
	// RECOMMENDATION: Load the first byte now so it's ready for the first clock pulse
	// if(image_len > 0) {
	//	spi_write(SPI, image_buffer[0], 0, 0);
	//	transfer_index++;
	//	transfer_len--;
	//}
}

//set necessary parameters to prepare for SPI the SPI transfer

// write a command to theESP32 and waits either for an ack (via the command
// complete pin) or a timeout. The timeout can be created by setting the gloval variable
// counts to zerro, which will automatically increment every second and waiting while counts < cnt

//
void write_wifi_command(char* comm, uint8_t cnt){
	counts = 0;
	wifi_comm_success = false;
	
	while(*comm != '\0'){
		usart_write(WIFI_USART, *comm);
		comm++;
	}
	usart_write(WIFI_USART, '\r');
	usart_write(WIFI_USART, '\n');
	
	while(!wifi_comm_success && (counts < cnt)) {
		// wait
		// Do I have to put somethinig in this while loop as a placeholder
	}
	
}

// writes an image from teh SAM4S8B to the ESP32
// if the image lenght is zero, return
// otherwisein
// 1. configure the SPI interface to be ready for a transfer by setting its parameters appropriately
// 2. issue the command "image_transfer xxx" where xxx is replaced by the length of the image you
// want to transfer
// 3. the ESP32 will then set the "command complete" pin low and begin transferring the image of spi
// 4. after the image is done sending, the ESP 32 will ste the "command complete" pin high.
// The MCu should sense this and move on

void write_image_to_web(void){
	
	char cmd_buffer[50];
	
	prepare_spi_transfer();
	
	sprintf(cmd_buffer, "image_transfer %d", transfer_len);
	
	// Send the command and wait up to 5 seconds for ack
	write_wifi_command(cmd_buffer, 5);

	// 3. Wait for ESP32 to set pin LOW (Transfer Starting)
	// We poll the pin until it reads 0.
	while(pio_get(WIFI_COMM_PIO, PIO_TYPE_PIO_INPUT, WIFI_COMM_PIN_NUM) == 1);

	// 4. Wait for ESP32 to set pin HIGH (Transfer Finished)
	// The actual data sending happens in the background via the SPI interrupt (wifi_spi_handler)
	// We just wait here until the ESP32 says "I got it all".
	while(pio_get(WIFI_COMM_PIO, PIO_TYPE_PIO_INPUT, WIFI_COMM_PIN_NUM) == 0);
	
	
}
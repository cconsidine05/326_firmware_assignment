/*
 * wifi.h
 *
 * Created: 2/27/2026 12:29:54 PM
 *  Author: ccons
 */ 


#ifndef WIFI_H_
#define WIFI_H_

#define WIFI_USART				USART0
#define WIFI_USART_ID			ID_USART0
#define WIFI_USART_BAUDRATE		115200
#define WIFI_USART_HANDLER		USART0_Handler
#define WIFI_USART_IRQn			USART0_IRQn
#define WIFI_USART_CHAR_LENGTH	US_MR_CHRL_8_BIT


#define MAX_INPUT_WIFI 1000
extern volatile char input_line_wifi[MAX_INPUT_WIFI];
extern volatile uint32_t received_byte_wifi;
extern volatile bool new_rx_wifi;
extern volatile unsigned int input_pos_wifi;
extern volatile bool wifi_comm_success;

#define PIN_USART0_TXD_FLAGS			(PIO_PERIPH_A | PIO_PULLUP)

// all interrupt mask

#define ALL_INTERRUPT_MASK 0xffffffff

#define WIFI_COMM_PIN_NUM			PIO_PB10
#define WIFI_COMM_PIO				PIOB
#define WIFI_COMM_ID				ID_PIOB
#define WIFI_COMM_ATTR				PIO_IT_RISE_EDGE

#define WIFI_PROV_PIN				PIO_PB2_IDX
#define WIFI_PROV_PIN_ID			ID_PIOB
#define WIFI_PROV_PIO				PIOB
#define WIFI_PROV_MASK				PIO_PB2 
#define WIFI_PROV_ATTR				(PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_FALL_EDGE)

//#define PUSH_BUTTON_ID ID_PIOA
//#define PUSH_BUTTON_PIO  PIOA
//#define PUSH_BUTTON_PIN_MSK PIO_PA23
//#define PUSH_BUTTON_ATTR PIO_PULLUP | PIO_DEBOUNCE | PIO_IT_RISE_EDGE

#define PIN_USART0_RXD_IDX PIO_PA5_IDX
#define PIN_USART0_TXD_IDX PIO_PA6_IDX

#define PIN_USART0_RXD_FLAGS	(PIO_PERIPH_A | PIO_PULLUP)
#define PIN_USART0_TXD_FLAGS	(PIO_PERIPH_A | PIO_PULLUP)

#define SPI_MISO_GPIO			(PIO_PA12_IDX)
#define SPI_MISO_FLAGS			(PIO_PERIPH_A | PIO_DEFAULT)

#define SPI_MOSI_GPIO			(PIO_PA13_IDX)
#define SPI_MOSI_FLAGS			(PIO_PERIPH_A | PIO_DEFAULT)

#define SPI_SPK_GPIO			(PIO_PA14_IDX)
#define SPI_SPCK_FLAGS			(PIO_PERIPH_A | PIO_DEFAULT)

// Just added these things now:
extern volatile uint32_t transfer_len;
extern volatile uint32_t transfer_index;
extern volatile bool wifi_provision_flag;

// handler for incoming data from the WiFi. 
// should call process incoming_byte when it arrives
void wifi_usart_handler(void);

// stores every incoming byte from the ESP32 in a buffer
void process_incoming_byte_wifi(uint8_t byte);

extern volatile uint32_t counts;
extern volatile uint32_t image_len;
extern volatile uint8_t image_buffer[];

// handler for command complete rising edge interrupt from ESP 32. When it is triggered
// it is time to process the result from the esp32

void wifi_command_response_handler(uint32_t ul_id, uint32_t ul_mask);

// processes the reponse of the ESP32, which should be stored in the bugger filled 
// by process incoming byte wifi. this processing should be looking for cetain responses
// that the ESP32 should give, such as "SUCCESS" when "test" is sent to it
void process_data_wifi(void);

// handler for button to initiate procisioning mode of the ESP32
// should set a flag indicating a request to initiate provisinoing mode

void wifi_provision_handler(uint32_t ul_id, uint32_t ul_mask);

// handler for peripheral mode interrupts on SPI bus. When the ESP32 SPi
// controller requests data, this interrupt should send one byte of the image
// at a time
void wifi_spi_handler(void);

// configuration of USART port used to communicate with the ESP32

void configure_usart_wifi(void);

// configuraiton of the "command complete" rising edge interrupt 

void configure_wifi_comm_pin(void);

//configuraiton of button interrupt to initiate provisioning mode

void configure_wifi_provision_pin(void);

// configuration of SPI port and interrupts used to send images to the ESP32

void configure_spi(void);

// initialize teh SPI port as a peripheral (slave) device. Note: 
// the embedded instury is trying to phase out the "master/slave"

void spi_peripheral_initialize(void);

//set necessary parameters to prepare for SPI the SPI transfer

// write a command to theESP32 and waits either for an ack (via the command
// complete pin) or a timeout. The timeout can be created by setting the gloval variable
// counts to zerro, which will automatically increment every second and waiting while counts < cnt

void prepare_spi_transfer(void);

void write_wifi_command(char* comm, uint8_t cnt);

// writes an image from teh SAM4S8B to the ESP32
// if the image lenght is zero, return
// otherwise
// 1. configure the SPI interface to be ready for a transfer by setting its parameters appropriately
// 2. issue the command "image_transfer xxx" where xxx is replaced by the length of the image you 
// want to transfer
// 3. the ESP32 will then set the "command complete" pin low and begin transferring the image of spi
// 4. after the image is done sending, the ESP 32 will ste the "command complete" pin high. 
// The MCu should sense this and move on 

void write_image_to_web(void);




#endif /* WIFI_H_ */
/*
 * camera.h
 *
 * Created: 2/23/2026 10:25:32 AM
 *  Author: laptop
 */ 


#ifndef CAMERA_H_
#define CAMERA_H_

#include <asf.h>

// void vsync_handler(uint32_t ul_id, uint32_t ul_mask);

// void init_vsync_interrupts(void);

void configure_twi(void);

void pio_capture_init(Pio *p_pio, uint32_t ul_id);

uint8_t pio_capture_to_buffer(Pio *p_pio, uint8_t *uc_buf, uint32_t ul_size);

void init_camera(void);

void configure_camera(void);

// uint8_t start_capture(void);

uint8_t find_image_len(void);

#endif /* CAMERA_H_ */
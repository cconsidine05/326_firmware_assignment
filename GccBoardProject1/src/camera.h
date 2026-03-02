/*
 * camera.h
 *
 * Created: 2/23/2026 10:25:32 AM
 *  Author: laptop
 */ 


#ifndef CAMERA_H_
#define CAMERA_H_

#include <asf.h>

void init_vsync_interrupts(void);

void pio_capture_init(Pio *p_pio, uint32_t ul_id);

void init_camera(void);

void configure_camera(void);

uint8_t start_capture(void);

#endif /* CAMERA_H_ */
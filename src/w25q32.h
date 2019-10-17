#ifndef __W25Q32_H__
#define __W25Q32_H__

/*
 * cs    GPIO05  D1
 * mosi  GPIO04  D2
 * sck   GPIO00  D3
 * miso  GPIO02  D4
 * */

#include <stdint.h>
#include "c_types.h"
#include "eagle_soc.h"
#include "gpio.h"
#include "osapi.h"
#include "mem.h"

#define CS 5
#define MOSI 4
#define SCLK 0
#define MISO 2

typedef struct unique_id {
	uint32_t MSB;
	uint32_t LSB;
} UID;


void spi_io_init();

uint8_t read_state_reg();

void write_enable();
void write_disable();

uint16_t read_device_id();
uint32_t read_jedec_id();
UID read_unique_id();

uint32_t w25q32_read(uint8_t *buffer, uint32_t size, uint32_t address);

uint8_t w25q32_write_page(uint8_t *buffer, uint32_t size, uint32_t address);
uint8_t w25q32_write_multipage(uint8_t *buffer, uint32_t size, uint32_t address);

uint8_t sector_erase(uint32_t address);
uint8_t block_erase_32k(uint32_t address);
uint8_t block_erase_64k(uint32_t address);
uint8_t chip_erase();

#endif

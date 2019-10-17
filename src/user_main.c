#include "user_main.h"

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void) {
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;
        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;
        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;
        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }
    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void) {
}

void ICACHE_FLASH_ATTR user_init(void) {

	uint8_t state_reg = 0xFF;
	uint16_t dev_id = 0x00;
	uint32_t jedec_id = 0x00;
	UID uid;
	uint32_t i = 0, j = 1;
	uint8_t *buffer = NULL;
	uint32_t size = 64;

	uint8_t data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
						0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07
					};

	buffer = (uint8_t *)os_malloc(sizeof(uint8_t) * size);


	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_printf("BIT_RATE_115200\n");

	//关闭wifi
	wifi_set_opmode(NULL_MODE);
	os_printf("NULL_MODE\n");

	spi_io_init();
	os_printf("spi_io_init\n");

	write_disable();
	state_reg = read_state_reg();
	os_printf("before write enable state_reg: 0x%x\n", state_reg);

	write_enable();
	state_reg = read_state_reg();
	os_printf("after write enable state_reg: 0x%x\n", state_reg);

	dev_id = read_device_id();
	os_printf("dev_id: 0x%x\n", dev_id);

	jedec_id = read_jedec_id();
	os_printf("jedec_id: 0x%x\n", jedec_id);

	uid = read_unique_id();
	os_printf("unique_id: 0x%x\n", uid.MSB);
	os_printf("unique_id: 0x%x\n", uid.LSB);

	//state_reg = sector_erase(0x00);
	//os_printf("after sector_erase state_reg: 0x%x\n", state_reg);

	os_printf("init read:\n");
	w25q32_read(buffer, size, 0x0000);
	for(i = 0; i < size; i++) {
		if(*(buffer + i) < 0x10) {
			os_printf("0x0%x ", *(buffer + i));
		}else {
			os_printf("0x%x ", *(buffer + i));
		}

		if(j % 8 == 0) {
			os_printf("\n");
		}

		j++;
	}

	/*
	w25q32_write_page(data, 24, 0x28);
	os_printf("write data\n");

	os_printf("after write :\n");
	w25q32_read(buffer, size, 0x0000);
	for(i = 0, j = 1; i < size; i++) {
		if(*(buffer + i) < 0x10) {
			os_printf("0x0%x ", *(buffer + i));
		}else {
			os_printf("0x%x ", *(buffer + i));
		}

		if(j % 8 == 0) {
			os_printf("\n");
		}
		j++;
	}
	*/
}

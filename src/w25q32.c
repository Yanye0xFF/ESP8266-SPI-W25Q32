#include "w25q32.h"

void spi_io_init() {
	//CS
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO5_U);
	//SCLK
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);
	//MOSI
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
	PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO0_U);
	//MISO输入模式,不需要上拉
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
	PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO5_U);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(2));
}

/**
 * 写字节,MSB-->LSB
 * @param data 写入数据
 * */
void spi_write_byte(uint8_t  data) {
	sint8_t i;
	uint8_t tmp;
	for(i = 7; i >= 0; i--) {
		tmp = (data >> i) & 0x1;
		GPIO_OUTPUT_SET(MOSI, tmp);
		GPIO_OUTPUT_SET(SCLK, 0);
		GPIO_OUTPUT_SET(SCLK, 1);
	}
	GPIO_OUTPUT_SET(MOSI, 1);
}

/**
 * 读字节,LSB-->MSB
 * @return 读出的字节数据
 * */
uint8_t spi_read_byte() {
	uint8_t data = 0x00, i;
	for(i = 8; i; i--) {
		GPIO_OUTPUT_SET(SCLK, 0);
		GPIO_OUTPUT_SET(SCLK, 1);
		data <<= 1;
		if(GPIO_INPUT_GET(MISO)){
			data |= 0x1;
		}
	}
	GPIO_OUTPUT_SET(SCLK, 0);
	return data;
}

/**
 * 读取状态寄存器
 * SPR RV TB BP2 BP1 BP0 WEL BUSY
 * SPR:默认0,状态寄存器保护位,配合WP使用
 * TB,BP2,BP1,BP0:FLASH区域写保护设置
 * WEL:写使能锁定(1,可写入;0,不可写入)
 * BUSY:忙标记位(1,忙;0,空闲)
 * @return state (8bit)
 * */
uint8_t read_state_reg() {
	uint8_t state = 0xFF;
	GPIO_OUTPUT_SET(CS, 0);
	spi_write_byte(0x05);
	state = spi_read_byte();
	GPIO_OUTPUT_SET(CS, 1);
	return state;
}

/**
 * 写使能
 * */
void write_enable() {
	GPIO_OUTPUT_SET(CS, 0);
	spi_write_byte(0x06);
	GPIO_OUTPUT_SET(CS, 1);
}

/**
 * 写禁止
 * */
void write_disable() {
	GPIO_OUTPUT_SET(CS, 0);
	spi_write_byte(0x04);
	GPIO_OUTPUT_SET(CS, 1);
}

/**
 * 读取设备ID
 * w25q32: 0xEF15
 * W25q16: 0XEF14
 * (M7-M0) 生产厂商                 MSB
 * (ID15-ID8) 存储器类型      LSB
 * @return dev_id (16bit)
 * */
uint16_t read_device_id() {
	uint16_t dev_id = 0x00;

	GPIO_OUTPUT_SET(CS, 0);
	spi_write_byte(0x90);
	spi_write_byte(0x00);
	spi_write_byte(0x00);
	spi_write_byte(0x00);

	dev_id = spi_read_byte();
	dev_id <<= 8;
	dev_id |= spi_read_byte();

	GPIO_OUTPUT_SET(CS, 1);
	return dev_id;
}

/**
 * 读取器件ID
 * W25Q32: EF4016
 * W25Q64: EF4017
 * W25Q128: EF4018
 * (M7-M0) 生产厂商                 MSB
 * (ID15-ID8) 存储器类型       |
 * (ID7-ID0) 容量                   LSB
 * @return jedec_id (24bit)
 * */
uint32_t read_jedec_id() {
	uint32_t jedec_id = 0x00;

	GPIO_OUTPUT_SET(CS, 0);
	spi_write_byte(0x9F);

	jedec_id = spi_read_byte() << 16;
	jedec_id |= spi_read_byte() << 8;
	jedec_id |= spi_read_byte();

	GPIO_OUTPUT_SET(CS, 1);
	return jedec_id;
}

UID read_unique_id() {

	UID uid;

	GPIO_OUTPUT_SET(CS, 0);

	spi_write_byte(0x4B);
	spi_write_byte(0x00);
	spi_write_byte(0x00);
	spi_write_byte(0x00);
	spi_write_byte(0x00);

	uid.MSB = spi_read_byte() << 24;
	uid.MSB |= spi_read_byte() << 16;
	uid.MSB |= spi_read_byte() << 8;
	uid.MSB |= spi_read_byte();

	uid.LSB |= spi_read_byte() << 24;
	uid.LSB |= spi_read_byte() << 16;
	uid.LSB |= spi_read_byte() << 8;
	uid.LSB |= spi_read_byte();

	GPIO_OUTPUT_SET(CS, 1);
	return uid;
}


uint8_t wait_busy(uint32_t timeout) {
	uint8_t state = 0xFF;
	GPIO_OUTPUT_SET(CS, 0);
	do{
		spi_write_byte(0x05);
		state = spi_read_byte();
		timeout--;
	}while((state & 0x1) && timeout);
	GPIO_OUTPUT_SET(CS, 1);
	return state;
}

/**
 * 整个芯片擦除,擦除完成后为FF
 * W25Q16:25s
 * W25Q32:40s
 * W25Q64:40s
 * @return state register
 * */
uint8_t chip_erase() {
	uint8_t state = 0xFF;
	write_enable();

	GPIO_OUTPUT_SET(CS, 0);
	spi_write_byte(0xC7);
	GPIO_OUTPUT_SET(CS, 1);

	state = wait_busy(65535);
	return state;
}

uint8_t w25q32_erase(uint8_t cmd, uint32_t address) {
	uint8_t state = 0xFF;
	write_enable();
	state = wait_busy(65535);
	if((state & 0x1) == 0) {
		GPIO_OUTPUT_SET(CS, 0);
		spi_write_byte(cmd);
		spi_write_byte((address >> 16) & 0xFF);
		spi_write_byte((address >> 8) & 0xFF);
		spi_write_byte(address & 0xFF);

		GPIO_OUTPUT_SET(CS, 1);
		state = wait_busy(65535);
	}
	return state;
}

/**
 * 扇区擦除 4KB, Tmin = 150ms
 * @param address 扇区起始地址
 * @return state register
 * */
uint8_t sector_erase(uint32_t address) {
	return w25q32_erase(0x20, address);
}

/**
 * 块擦除 32KB
 * @param address 32K块起始地址
 * @return state register
 * */
uint8_t block_erase_32k(uint32_t address) {
	return w25q32_erase(0x52, address);
}

/**
 * 块擦除 64KB
 * @param address 64K块起始地址
 * @return state register
 * */
uint8_t block_erase_64k(uint32_t address) {
	return w25q32_erase(0xD8, address);
}

uint32_t w25q32_read(uint8_t *buffer, uint32_t size, uint32_t address) {
	uint32_t i = 0;

	if(buffer == NULL || size <= 0) {
		return i;
	}
	wait_busy(65535);

	GPIO_OUTPUT_SET(CS, 0);

	spi_write_byte(0x03);
	spi_write_byte((address >> 16) & 0xFF);
	spi_write_byte((address >> 8) & 0xFF);
	spi_write_byte(address & 0xFF);

	do{
		*buffer = spi_read_byte();
		buffer++;
		i++;
		size--;
	}while(size);

	GPIO_OUTPUT_SET(CS, 1);
	return i;
}

/**
 * 写一页数据,最大256bytes
 * 由于超出后会回到初始地址覆盖数据,故内部限制MAX size = 256
 * @param buffer 写入数据缓冲区
 * @param size 实际写入数据长度
 * @param address 写入地址
 * */
uint8_t w25q32_write_page(uint8_t *buffer, uint32_t size, uint32_t address) {
	uint8_t state = 0xFF;

	if(buffer == NULL || size <= 0) {
		return 0x00;
	}

	if(size > 256) {
		size = 256;
	}

	write_enable();
	wait_busy(65535);

	GPIO_OUTPUT_SET(CS, 0);

	spi_write_byte(0x02);
	spi_write_byte((address >> 16) & 0xFF);
	spi_write_byte((address >> 8) & 0xFF);
	spi_write_byte(address & 0xFF);

	do {
		spi_write_byte(*buffer);
		buffer++;
		size--;
	}while(size);

	GPIO_OUTPUT_SET(CS, 1);

	state = wait_busy(65535);
	return state;
}

uint8_t w25q32_write_multipage(uint8_t *buffer, uint32_t size, uint32_t address) {
	uint8_t state = 0xFF;

	if(buffer == NULL || size <= 0) {
		return 0x00;
	}
    //TODO 
	return state;
}


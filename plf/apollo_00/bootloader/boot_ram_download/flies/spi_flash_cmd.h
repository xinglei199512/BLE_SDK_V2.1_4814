#ifndef SPI_FLASH_CMD_H_
#define SPI_FLASH_CMD_H_


enum flash_commands{
	FLASH_WRITE_STATUS_REGISTER_1 = 0x01,
	FLASH_WRITE_STATUS_REGISTER_2 = 0x31,
	FLASH_WRITE_STATUS_REGISTER_3 = 0x11,
	FLASH_PAGE_PROGRAM = 0x02,
	FLASH_READ_DATA_BYTES = 0x03,
	FLASH_WRITE_DISABLE = 0x04,
	FLASH_READ_STATUS_REGISTER_1 = 0x05,
	FLASH_READ_STATUS_REGISTER_2 = 0x35,
	FLASH_READ_STATUS_REGISTER_3 = 0x15,
	FLASH_WRITE_ENABLE = 0x06,
	FLASH_DUAL_OUTPUT_FAST_READ = 0x3B,
	FLASH_QUAD_OUTPUT_FAST_READ = 0x6B,
	FLASH_DUAL_IO_FAST_READ = 0xBB,
	FLASH_QUAD_IO_FAST_READ = 0xEB,
	FLASH_SECTOR_ERASE = 0x20,
	FLASH_32KB_BLOCK_ERASE = 0x52,
	FLASH_64KB_BLOCK_ERASE = 0xd8,
	FLASH_CHIP_ERASE = 0x60,
	FLASH_DEEP_POWER_DOWN = 0xB9,
	FLASH_RELEASE_POWER_DOWN = 0xAB,
	FLASH_ENABLE_RESET = 0x66,
	FLASH_READ_DEVICE_ID = 0x90,
	FLASH_READ_JEDEC_ID = 0x9f,
	FLASH_RESET_DEVICE = 0x99,
	FLASH_READ_SFDP = 0x5a,
};


#endif

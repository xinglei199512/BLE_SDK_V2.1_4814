#ifndef FLASH_BASE_H_
#define FLASH_BASE_H_
#include <stdint.h>
#include <stdbool.h>
#include "periph_error.h"
#include "app_qspi_wrapper.h"

periph_err_t flash_erase_operation_start(uint8_t cmd,uint32_t addr,bool whole_chip);
periph_err_t flash_program_operation_start(uint8_t cmd,uint32_t offset, uint32_t length, uint8_t *buffer);
periph_err_t flash_read_operation_start(uint8_t *cmd_buf,uint8_t cmd_len,uint8_t *data_buf,uint16_t data_len);
periph_err_t flash_write_operation_start(uint8_t *data,uint32_t length);
periph_err_t flash_write_no_dma_operation_start(uint8_t *data,uint16_t length);
periph_err_t flash_multi_read_32bits_operation_start(uint32_t *data,uint16_t length,uint32_t addr);
periph_err_t flash_write_status_reg_start(uint8_t cmd,uint8_t *data,uint8_t length);


#endif

#ifndef APP_QSPI_WRAPPER_H_
#define APP_QSPI_WRAPPER_H_
#include <stdint.h>
#include <stdbool.h>
#include "dw_apb_ssi_typedef.h"
#include "app_qspi.h"

void app_qspi_param_init_wrapper(multi_read_param_t *multi_read);

void app_qspi_init_wrapper(void);

periph_err_t app_qspi_std_write_wrapper(uint8_t *data,uint32_t length);

periph_err_t app_qspi_std_write_no_dma_wrapper(uint8_t *data,uint16_t length);

periph_err_t app_qspi_flash_program_wrapper(uint8_t cmd,uint32_t addr,uint8_t *data,uint32_t length);

periph_err_t app_qspi_std_read_wrapper(uint8_t *cmd_buf,uint8_t cmd_len,uint8_t *data_buf,uint16_t data_len);

periph_err_t app_qspi_multi_read_32bits_wrapper(uint32_t *data,uint16_t length,qspi_addr_data_t addr);

periph_err_t app_qspi_config_for_cache_wrapper(uint16_t cache_line_size);

uint8_t app_qspi_multi_read_cmd_get_wrapper(void);

uint32_t app_qspi_dr_reg_addr_for_cache_wrapper(void);

periph_err_t app_qspi_disable_for_cache_wrapper(void);


#endif

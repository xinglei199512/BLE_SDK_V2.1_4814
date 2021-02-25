#include <stdbool.h>
#include <stdarg.h>
#include "apollo_00.h"
#include "spi_flash_cmd.h"
#include "bx_dbg.h"
#include "flash_integration.h"
#include "flash_base.h"
#include "arch_init.h"
#include "compiler_flag.h"
#include "svcall.h"

SVCALL(SVCALL_FLASH_PROGRAM,periph_err_t,flash_program_svcall(uint8_t cmd,uint32_t offset, uint32_t length, uint8_t *buffer));
SVCALL(SVCALL_FLASH_ERASE,periph_err_t,flash_erase_svcall(uint8_t cmd,uint32_t addr,bool whole_chip));
SVCALL(SVCALL_FLASH_READ,periph_err_t,flash_read_svcall(uint8_t *cmd_buf,uint8_t cmd_len,uint8_t *data_buf,uint16_t data_len));
SVCALL(SVCALL_FLASH_MULTI_READ_32BITS,periph_err_t,flash_multi_read_32bits_svcall(uint32_t *data,uint16_t length,uint32_t addr));
SVCALL(SVCALL_FLASH_WRITE_STATUS_REG,periph_err_t,flash_write_status_reg_svcall(uint8_t cmd,uint8_t *data,uint8_t length));

#define FLASH_EXECUTE(return_val,op,...) \
    do{\
        if(__get_PRIMASK())\
        {\
            return_val = op##_execution(__VA_ARGS__);\
        }else\
        {\
            return_val = op##_svcall(__VA_ARGS__);\
        }\
    }while(0)

N_XIP_SECTION periph_err_t flash_program_execution(uint8_t cmd,uint32_t offset, uint32_t length, uint8_t *buffer)
{
    periph_err_t error;
    cache_disable();
    error = flash_program_operation_start(cmd, offset, length, buffer);
    cache_enable();
    return error;
}

periph_err_t flash_program_operation(uint8_t cmd,uint32_t offset, uint32_t length, uint8_t *buffer)
{
    periph_err_t error;
    FLASH_EXECUTE(error,flash_program,cmd,offset,length,buffer);
    return error;
}

N_XIP_SECTION periph_err_t flash_erase_execution(uint8_t cmd,uint32_t addr,bool whole_chip)
{
    periph_err_t error;
    cache_disable();
    error = flash_erase_operation_start(cmd, addr,whole_chip);
    cache_enable();
    return error;
}

periph_err_t flash_erase_operation(uint8_t cmd,uint32_t addr,bool whole_chip)
{
    periph_err_t error;
    FLASH_EXECUTE(error,flash_erase,cmd,addr,whole_chip);
    return error;
}

N_XIP_SECTION periph_err_t flash_read_execution(uint8_t *cmd_buf,uint8_t cmd_len,uint8_t *data_buf,uint16_t data_len)
{
    periph_err_t error;
    cache_disable();
    error = flash_read_operation_start(cmd_buf, cmd_len,data_buf,data_len);
    cache_enable();
    return error;
}

periph_err_t flash_read_operation(uint8_t *cmd_buf,uint8_t cmd_len,uint8_t *data_buf,uint16_t data_len)
{
    periph_err_t error;
    FLASH_EXECUTE(error,flash_read,cmd_buf,cmd_len,data_buf,data_len);
    return error;
}

N_XIP_SECTION periph_err_t flash_multi_read_32bits_execution(uint32_t *data,uint16_t length,uint32_t addr)
{
    periph_err_t error;
    cache_disable();
    error = flash_multi_read_32bits_operation_start(data,length,addr);
    cache_enable();
    return error;
}

periph_err_t flash_multi_read_32bits_operation(uint32_t *data,uint16_t length,uint32_t addr)
{
    periph_err_t error;
    FLASH_EXECUTE(error,flash_multi_read_32bits,data,length,addr);
    return error;
}

N_XIP_SECTION periph_err_t flash_write_status_reg_execution(uint8_t cmd,uint8_t *data,uint8_t length)
{
    periph_err_t error;
    cache_disable();
    error = flash_write_status_reg_start(cmd,data, length);
    cache_enable();
    return error;
}

periph_err_t flash_write_status_reg_operation(uint8_t cmd,uint8_t *data,uint8_t length)
{
    periph_err_t error;
    FLASH_EXECUTE(error,flash_write_status_reg,cmd,data,length);
    return error;
}

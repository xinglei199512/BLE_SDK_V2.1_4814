#include <string.h>
#include "flash_integration.h"
#include "flash_wrapper.h"
#include "flash_base.h"
#include "arch_init.h"
#include "spi_flash_cmd.h"
#include "compiler_flag.h"
#include "app_dmac.h"


static periph_err_t flash_multi_read_word(uint32_t offset,uint16_t length,uint32_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_multi_read(uint32_t offset,uint32_t length,uint8_t * buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_erase_security_reg(uint8_t reg_num)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_program_security_reg(uint8_t reg_num,uint16_t offset,uint16_t length,uint8_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_program(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_page_program(uint32_t offset,uint16_t length,uint8_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_erase(uint32_t offset,erase_t type)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_sfdp(uint32_t addr,uint8_t *buf,uint8_t buf_len)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_jedec_ptp(uint32_t *jedec_ptp)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_manufacturer_device_id(uint8_t *manufacturer_id,uint8_t *device_id)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_jedec_id(uint8_t *manufacturer_id,uint8_t *mem_type,uint8_t *capacity)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_memory_density(uint32_t *mem_density)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_reset()
{
    return PERIPH_NO_ERROR;
}

N_XIP_SECTION periph_err_t flash_wakeup()
{
    return PERIPH_NO_ERROR;
}

N_XIP_SECTION periph_err_t flash_deep_power_down()
{
    return PERIPH_NO_ERROR;
}

static periph_err_t flash_read_byte(uint32_t offset, uint16_t length, uint8_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_std_read(uint32_t offset,uint32_t length,uint8_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_security_reg(uint8_t reg_num,uint16_t offset,uint16_t length,uint8_t *buffer)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_read_status_reg(uint8_t cmd,uint8_t *data,uint8_t length)
{
    return PERIPH_NO_ERROR;
}

periph_err_t flash_write_status_reg(uint8_t cmd,uint8_t *data,uint8_t length)
{
    return PERIPH_NO_ERROR;
}

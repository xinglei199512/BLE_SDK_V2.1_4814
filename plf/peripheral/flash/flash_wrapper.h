/** @addtogroup FLASH
 *    @ingroup PERIPHERAL
 *
 *    All flash operation functions works in blocking mode, and the global interrupt is disabled when necessarily.
 *    So be aware of the time consumption of calling flash APIs. Programming a page of 256 bytes will typically 
 *    consume serveral hundreds of us. Erasing a 4KB Sector will typically consume several tens of ms. For detailed
 *    timing specification, please refer to the specification of the SPI Flash on PCB board.
 *  @{
 */
#ifndef FLASH_WRAPPER_H_
#define FLASH_WRAPPER_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "periph_error.h"
#define FLASH_TYPE_UNKNOWN             0
#define FLASH_PAGE_SIZE               256

/**@brief flash erase type */
typedef enum{
    Page_Erase,
    Sector_Erase,
    Block_32KB_Erase,
    Block_64KB_Erase,
    Chip_Erase,
}erase_t;

/**@brief flash reset command */
periph_err_t flash_reset(void);

/**@brief flash wakeup command */
periph_err_t flash_wakeup(void);

periph_err_t flash_read_manufacturer_device_id(uint8_t *manufacturer_id,uint8_t *device_id);

periph_err_t flash_read_jedec_id(uint8_t *manufacturer_id,uint8_t *mem_type,uint8_t *capacity);

periph_err_t flash_erase_security_reg(uint8_t reg_num);

periph_err_t flash_program_security_reg(uint8_t reg_num, uint16_t offset, uint16_t length, uint8_t * buffer);

periph_err_t flash_read_security_reg(uint8_t reg_num, uint16_t offset, uint16_t length, uint8_t * buffer);

periph_err_t flash_read_status_reg(uint8_t cmd,uint8_t *data,uint8_t length);

periph_err_t flash_write_status_reg(uint8_t cmd,uint8_t *data,uint8_t length);

/**
 * @brief flash read memory density
 */
periph_err_t flash_read_memory_density(uint32_t *mem_density);


/**
 * @brief flash read sfdp register
 * @param[in] addr sfdp register addr
 * @param[out] buf buffer where the content will be put
 * @param[in] buf_len buffer length
 */
periph_err_t flash_read_sfdp(uint32_t addr,uint8_t *buf,uint8_t buf_len);


/**
 * @brief   Erase a flash section.
 * @param[in] offset Starting offset from the beginning of the flash device to be erased
 * @param[in] type Erase command type
 * @return error_code
 */
periph_err_t flash_erase(uint32_t offset, erase_t type);

/**
 * @brief  Program flash.
 * @param[in] offset Starting offset from the beginning of the flash device to be programmed
 * @param[in] length Data length
 * @param[in] buffer Pointer on data to write
 * @return error_code
 */
periph_err_t flash_program(uint32_t offset, uint32_t length, uint8_t *buffer);

periph_err_t flash_page_program(uint32_t offset,uint16_t length,uint8_t *buffer);

/**
 * @brief   Read from flash.
 * @param[in] offset Starting offset from the beginning of the flash device to be read
 * @param[in] length Data length
 * @param[out] buffer buffer where data read from flash will be put
 * @return error_code
 */
periph_err_t flash_std_read(uint32_t offset,uint32_t length,uint8_t *buffer);

periph_err_t flash_multi_read(uint32_t offset, uint32_t length, uint8_t *buffer);


/** @brief Flash deep power down command. */
periph_err_t flash_deep_power_down(void);

void flash_test(void);

void flash_security_reg_test(void);

#endif // FLASH_H_
/** @}*/


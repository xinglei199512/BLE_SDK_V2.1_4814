#ifndef FLASH_H_
#define FLASH_H_

#include <stdint.h>              
#include <stdbool.h>

/**
 ****************************************************************************************
 * @addtogroup FLASH
 * @ingroup DRIVERS
 *
 * @brief Flash memory driver
 *
 * @{
 ****************************************************************************************
 */

typedef enum{
    Sector_Erase,
    Block_32KB_Erase,
    Block_64KB_Erase,
    Chip_Erase,
}erase_t;

/*
 * MACROS
 ****************************************************************************************
 */
extern void bx_delay_asm(unsigned int loop);
/**
 * 1. This macro should not be used until awo registers related to AHB_CLK have been set.
 * 2. Never set "a" as a value which is not an integer.
 * 3. Note that "a" should not result in overflow.
 * **/
#define BX_DELAY_US(a) bx_delay_asm((a)*8)

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
uint8_t flash_read_status_reg_2(void);
uint8_t flash_read_status_reg_1(void);
bool flash_quad_enable_check(void);
void flash_write_status_reg_2bytes(uint8_t reg1,uint8_t reg2);
void flash_quad_enable_01(void);
void flash_read_manufacturer_device_id(uint8_t *manufacturer_id,uint8_t *device_id);

void flash_read_jedec_id(uint8_t *manufacturer_id,uint8_t *mem_type,uint8_t *capacity);

void flash_reset(void);
void flash_wakeup(void);
uint32_t flash_read_memory_density(void);
void flash_read_sfdp(uint32_t addr,uint8_t *buf,uint8_t buf_len);

uint8_t flash_erase(uint32_t offset,erase_t type);

uint8_t flash_program(uint32_t offset, uint32_t length, uint8_t *buffer);

uint8_t flash_read(uint32_t offset, uint32_t length, uint8_t *buffer);

uint8_t flash_multi_read(uint32_t offset, uint32_t length, uint8_t *buffer,uint8_t type);

void flash_deep_power_down(void);

///
uint8_t flash_status_read(uint8_t reg_idx);
///
uint8_t flash_status_write(uint8_t reg,uint8_t value);
///
uint16_t flash_status_write_2bytes(uint8_t val_l,uint8_t val_h);

//uint32_t timeout  ms
//return  1 busy,  0 ok
uint8_t flash_operation_wait_time(uint32_t timeout);//unit = ms


/// @} FLASH

#endif // FLASH_H_

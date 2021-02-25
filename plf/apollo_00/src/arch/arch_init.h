#ifndef ARCH_INIT_H_
#define ARCH_INIT_H_
#include "rwip.h"
#include "app_qspi.h"
///indicate bootram_init has run in bootram
#define BOOTINIT_FLAG       0x3399

typedef struct{
    uint32_t ota_base;
    uint32_t data_base;
}app_info_t;

typedef struct
{
    uint8_t cmd;
    uint8_t quad_bit_offset:4,
            status_length:2;
}quad_status_rw_t;

typedef struct
{
    quad_status_rw_t read;
    quad_status_rw_t write;
}quad_enable_config_t;

typedef struct{
    uint16_t total_size_64k;
    multi_read_param_t multi_read_param;
    quad_enable_config_t quad_enable_config;
}flash_info_t;

void mpu_enable_for_xip_region(void);

void mpu_disable_for_xip_region(void);

void cache_config(void) ;

void cache_enable(void);

void cache_disable(void);

void soc_initialize(void);

void baremetal_blestack_init(void);

void soc_baremetal_loop(void);

const struct rwip_eif_api *serial_eif_get(void);
#endif

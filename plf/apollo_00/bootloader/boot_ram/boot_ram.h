#ifndef BOOT_RAM_H_
#define BOOT_RAM_H_
#include <stdint.h>
#include "boot_typedef.h"
#include "arch_init.h"
#define IMAGE_OFFSET_BASE 0x3000

typedef struct{
    boot_header_t header;
    app_info_t app;
    flash_info_t flash;
}boot_ram_header_t; 

#endif
#ifndef NVDS_IN_RAM_H_
#define NVDS_IN_RAM_H_
#include "rwip_config.h"
#include "nvds_typedef.h"
/*
 * STRUCT DECLARATIONS
 ****************************************************************************************
 */

uint8_t nvds_init(void);
uint8_t nvds_get(uint8_t tag, nvds_tag_len_t * lengthPtr, uint8_t *buf);
uint8_t nvds_del(uint8_t tag);
uint8_t nvds_put(uint8_t tag, nvds_tag_len_t length, uint8_t *buf);
void nvds_write_through(void);


#endif

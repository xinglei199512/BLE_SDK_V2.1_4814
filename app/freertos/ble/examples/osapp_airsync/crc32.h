#ifndef __CRC32_H__
#define __CRC32_H__
#include <stdlib.h>
#include "osapp_config.h"
#include "bx_log.h"

uint32_t crc32(uint32_t crc, const uint8_t *buf, int len);

#endif

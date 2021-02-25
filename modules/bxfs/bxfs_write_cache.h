#ifndef BXFS_WRITE_CACHE_H_
#define BXFS_WRITE_CACHE_H_
#include <stdint.h>

#define BXFS_WRITE_CACHE_SIZE 256
void bxfs_nvm_program(uint32_t offset,uint32_t length,uint8_t *buffer);
void bxfs_nvm_read_with_cache(uint32_t offset, uint32_t length, uint8_t *buffer);
void bxfs_nvm_write_through(void);

#endif

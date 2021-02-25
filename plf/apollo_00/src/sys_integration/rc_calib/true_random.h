#ifndef _TRUE_RANDOM_H_
#define _TRUE_RANDOM_H_

#include <stdint.h>




void hw_trng_get_numbers(uint8_t *buffer, uint8_t length);

uint32_t get_random_seed(void);
void generate_random_seed(void);


#endif


#ifndef OBFUSCATION_H_
#define OBFUSCATION_H_
#include <stdint.h>

#define OBFUSCATED_DATA_LENGTH                  6
#define PRIVACY_RANDOM_LENGTH                   7
#define NETWORK_OBFUSCATED_ZERO_CNT             5


typedef struct{
    uint8_t *privacy_key;
    uint8_t *privacy_random;
    uint32_t iv_index;
    uint8_t *src_data;
    uint8_t *rslt;
}obfuscation_param_t;

void obfuscation_start(obfuscation_param_t *ptr,void (*callback)(obfuscation_param_t *,void *,uint8_t));

#endif

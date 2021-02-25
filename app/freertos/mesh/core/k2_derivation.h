#ifndef K2_DERIVATION_H_
#define K2_DERIVATION_H_
#include <stdint.h>
#include <stdbool.h>
#include "sdk_mesh_definitions.h"
#include "gap.h"



typedef struct
{
    uint16_t friend_cnt;
    uint16_t lpn_cnt;
    uint16_t friend_addr;
    uint16_t lpn_addr;
}k2_friendship_param_t;

typedef struct
{
    uint8_t *n;
    k2_friendship_param_t friend;
    security_credentials_t *rslt;
    bool master;
}k2_derivation_buf_t;

extern uint8_t salt_for_k2[GAP_KEY_LEN];;

void k2_derivation_start(k2_derivation_buf_t *buf,void (*cb)(k2_derivation_buf_t *,void *,uint8_t));



#endif


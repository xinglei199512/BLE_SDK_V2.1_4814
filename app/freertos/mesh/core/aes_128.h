#ifndef AES_128_H_
#define AES_128_H_
#include <stdint.h>
#include "ke_msg.h"
#include "gapm_task.h"

typedef struct
{
    uint8_t const *key;
    uint8_t const *plain;
    uint8_t *encrypted;
}aes_128_param_t;

void aes_128_start(aes_128_param_t *param,void (*callback)(aes_128_param_t *,void *,uint8_t));

void osapp_gapm_use_enc_block_ind(ke_msg_id_t const msgid, struct gapm_use_enc_block_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id);
#endif

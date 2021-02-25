#include <stdbool.h>
#include <string.h>
#include "obfuscation.h"
#include "aes_128.h"
#include "queued_async_framework.h"
#include "gap.h"
#include "osapp_utils.h"
#include "stack_mem_cfg.h"

static uint8_t obfuscation_data_in[GAP_KEY_LEN];
static uint8_t obfuscation_data_out[GAP_KEY_LEN];


static void obfuscation_calc(obfuscation_param_t *ptr);
QUEUED_ASYNC_PROCESS_SPECIFIC_CALLBACK(obfuscation_async_process, OBFUSCATION_BUF_QUEUE_LENGTH,\
    obfuscation_param_t, obfuscation_calc);

static void obfuscation_pecb_callback(aes_128_param_t *aes_128,void *dummy,uint8_t status)
{
    obfuscation_param_t *param = queued_async_process_get_current(&obfuscation_async_process);
    uint8_t i;
    for(i=0;i<OBFUSCATED_DATA_LENGTH;++i)
    {
        param->rslt[i] = param->src_data[i]^obfuscation_data_out[i];
    }
    queued_async_process_end(&obfuscation_async_process,NULL, status);
}

static void obfuscation_calc(obfuscation_param_t *ptr)
{
    memset(obfuscation_data_in,0,NETWORK_OBFUSCATED_ZERO_CNT);
    memcpy_rev(&obfuscation_data_in[NETWORK_OBFUSCATED_ZERO_CNT],&ptr->iv_index,sizeof(uint32_t));
    memcpy(&obfuscation_data_in[NETWORK_OBFUSCATED_ZERO_CNT+sizeof(uint32_t)],ptr->privacy_random,PRIVACY_RANDOM_LENGTH);
    aes_128_param_t aes_128 = 
    {
        .key = ptr->privacy_key,
        .plain = obfuscation_data_in,
        .encrypted = obfuscation_data_out,
    };
    aes_128_start(&aes_128,obfuscation_pecb_callback);
}

void obfuscation_start(obfuscation_param_t *ptr,void (*callback)(obfuscation_param_t *,void *,uint8_t))
{
    queued_async_process_start(&obfuscation_async_process,ptr, (queued_async_callback_t)callback);
}



#include "aes_128.h"
#include "co_utils.h"
#include "osapp_utils.h"
#include "gapm_task.h"
#include "osapp_task.h"
#include "queued_async_framework.h"
#include "stack_mem_cfg.h"
#include "gapm_task.h"
#include "ble_task.h"
#include "osapp_config.h"

static void aes_128_calc(aes_128_param_t *aes);
QUEUED_ASYNC_PROCESS_SPECIFIC_CALLBACK(aes_128_async_process, AES_128_QUEUE_LENGTH, aes_128_param_t, aes_128_calc);
static int32_t osapp_gapm_use_enc_block_cmd(uint8_t const *key,uint8_t const *data)
{
    struct gapm_use_enc_block_cmd *cmd = AHI_MSG_ALLOC(GAPM_USE_ENC_BLOCK_CMD,TASK_ID_GAPM,gapm_use_enc_block_cmd);
    cmd->operation = GAPM_USE_ENC_BLOCK;
    memcpy_rev(cmd->operand_1,key,sizeof(cmd->operand_1));
    memcpy_rev(cmd->operand_2,data,sizeof(cmd->operand_2));
    return osapp_msg_build_send(cmd,sizeof(struct gapm_use_enc_block_cmd));
}

static void aes_128_calc(aes_128_param_t *aes)
{
    osapp_gapm_use_enc_block_cmd(aes->key,aes->plain);
}

void aes_128_start(aes_128_param_t *param,void (*callback)(aes_128_param_t *,void *,uint8_t))
{
    queued_async_process_start(&aes_128_async_process,param,(queued_async_callback_t)callback);
}

void osapp_gapm_use_enc_block_ind(ke_msg_id_t const msgid, struct gapm_use_enc_block_ind const *param,ke_task_id_t const dest_id,ke_task_id_t const src_id)
{
    aes_128_param_t *buf = queued_async_process_get_current(&aes_128_async_process);
    memcpy_rev(buf->encrypted,param,GAP_KEY_LEN);
    queued_async_process_end(&aes_128_async_process,NULL,0);
}

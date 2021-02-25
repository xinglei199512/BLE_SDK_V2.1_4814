#include "access_rx_process.h"
#include "access_pdu_decrypt.h"
#include "log.h"
#include "aes_ccm_cmac.h"
#include "mesh_env.h"
#include "stack_mem_cfg.h"
#include "upper_rx_process.h"
#include "model_rx.h"

static uint8_t *decrypted_buf;

static void access_rx_process_complete(upper_pdu_rx_t **ptr,void *param,uint8_t status);
static void access_rx_pre_process(upper_pdu_rx_t **);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(access_rx_async_process, ACCESS_RX_PROCESS_BUF_QUEUE_LENGTH,
    upper_pdu_rx_t *, access_rx_pre_process, access_rx_process_complete);


ble_txrx_time_t access_rx_get_rx_time(access_pdu_rx_t *ptr)
{
    return ptr->uppdu->rx_time;
}

uint16_t access_get_pdu_src_addr(access_pdu_rx_t * access_pdu)
{
    return access_pdu->uppdu->src_addr;
}

uint16_t access_get_pdu_dst_addr(access_pdu_rx_t * access_pdu)
{
    return access_pdu->uppdu->dst_addr;
}

virt_addr_mngt_t *  access_get_pdu_virt_addr(access_pdu_rx_t * access_pdu)
{
    return access_pdu->param->cryptic.virt_addr;
}

uint8_t * access_get_pdu_payload(access_pdu_rx_t * access_pdu)
{
    return access_pdu->access_payload;
}

uint16_t access_get_pdu_payload_length(access_pdu_rx_t * access_pdu)
{
    return access_pdu->uppdu->total_length;
}

uint16_t access_get_pdu_mic_length(access_pdu_rx_t * access_pdu)
{
    return access_pdu->uppdu->head.access.szmic?TRANSMIC_64BIT:TRANSMIC_32BIT;
}


uint8_t  access_get_pdu_op_head(access_pdu_rx_t * access_pdu)
{
    return *((uint8_t *)access_pdu->access_payload);
}

lower_pdu_head_info_u*  access_get_pdu_type_header(access_pdu_rx_t * access_pdu)
{
    return &access_pdu->uppdu->head;
}

uint8_t  access_is_access_pdu_type(access_pdu_rx_t * access_pdu)
{
    return access_pdu->uppdu->head.access.akf;
}



app_key_box_t*  access_get_pdu_app_keybox(access_pdu_rx_t * access_pdu)
{
    return access_pdu->param->cryptic.appkey_box;
}

app_key_t*  access_get_pdu_app_key(access_pdu_rx_t * access_pdu)
{
    return access_pdu->param->cryptic.appkey;
}

uint16_t  access_get_pdu_netkey_global_index(access_pdu_rx_t * access_pdu)
{
    return access_pdu->uppdu->netkey->global_idx;
}

uint16_t  access_get_pdu_appkey_global_index(access_pdu_rx_t * access_pdu)
{
    return access_pdu->param->cryptic.appkey->global_idx;
}


uint16_t get_access_pdu_rx_payload_size(access_pdu_rx_t *pdu)
{
    uint8_t opcode_size = 0;
    if((access_get_pdu_op_head(pdu) & 0xc0) == 0x00) {
        opcode_size += 1;
    }else if((access_get_pdu_op_head(pdu) & 0xc0) == 0x80) {
        opcode_size += 2;
    }else {
        opcode_size += 3;
    }

    return (access_get_pdu_payload_length(pdu) - access_get_pdu_mic_length(pdu) - opcode_size);
}



static void access_rx_process_complete(upper_pdu_rx_t **ptr,void *param,uint8_t status)
{
    upper_pdu_rx_t *upper_pdu = *ptr;
    upper_rx_pdu_free(upper_pdu);
}

static void access_decrypt_done(uint8_t status,access_pdu_decrypt_callback_param_t *param)
{
    upper_pdu_rx_t **ptr = queued_async_process_get_current(&access_rx_async_process);
    upper_pdu_rx_t *pdu = *ptr;
    if(status == DECRYPT_SUCCESSFUL)
    {
//        LOG(LOG_LVL_WARN,"access timer out %d\n",pdu->rx_time.time_cnt);
        deliver_to_model(pdu,decrypted_buf,param);       
    }else
    {
        LOG(LOG_LVL_WARN,"access decrypt fail\n");
    }
    mesh_free(decrypted_buf);
    queued_async_process_end(&access_rx_async_process,NULL,status);
}

static void access_rx_pre_process(upper_pdu_rx_t **ptr)
{
    upper_pdu_rx_t *pdu = *ptr;
    decrypted_buf = mesh_alloc(pdu->total_length);
    BX_ASSERT(decrypted_buf);
    access_pdu_decrypt_start(pdu,decrypted_buf,access_decrypt_done);
}

void access_rx_process_start(upper_pdu_rx_t *buf)
{
//    LOG(LOG_LVL_WARN,"access timer in %d\n",buf->rx_time.time_cnt);
    queued_async_process_start(&access_rx_async_process, &buf, NULL);
}

#include "access_tx_process.h"
#include "upper_tx_process.h"
#include "static_buffer.h"
#include "log.h"
#include "aes_ccm_cmac.h"
#include "access_pdu_encrypt.h"
#include "mesh_iv_operation_ex.h"
#include "mesh_env.h"
#include "stack_mem_cfg.h"
#include "mesh_reset_database.h"
#include "app_keys_dm.h"
#include "mesh_core_api.h"
#include "config_server.h"
#include "low_power.h"

#define VIRT_GROUP_REPLY_DELAY_MAX 500
#define UNICART_REPLY_DELAY_MAX 50
#define ACCESS_REPLY_DELAY_MIN 20

uint8_t access_max_used_num = 0;


static void access_tx_process_done(upper_tx_env_t **ptr,void *param,uint8_t status);
static void access_tx_pre_process(upper_tx_env_t **);
QUEUED_ASYNC_PROCESS_COMMON_CALLBACK(access_tx_async_process, ACCESS_TX_BUF_QUEUE_LENGTH,
    upper_tx_env_t *, access_tx_pre_process, access_tx_process_done);

DEF_ARRAY_BUF(access_tx_pdu_buf, access_pdu_tx_t, ACCESS_TX_PDU_BUF_SIZE);

static access_pdu_tx_t *access_tx_pdu_buf_alloc()
{
    return array_buf_alloc(&access_tx_pdu_buf);
}

static void access_tx_pdu_buf_release(access_pdu_tx_t *ptr)
{
    array_buf_release(&access_tx_pdu_buf,ptr);
}

static access_pdu_tx_t *access_tx_pdu_alloc(uint16_t pdu_length,bool seg,uint8_t szmic)
{
    access_pdu_tx_t *ptr = access_tx_pdu_buf_alloc();
    if(ptr)
    {
        uint8_t mic_length;
        if(seg == false && pdu_length <= 11)
        {
            ptr->pdu_base.seg = 0;
            ptr->szmic = 0;
            mic_length = TRANSMIC_32BIT;
        }else
        {
            ptr->pdu_base.seg = 1;
            ptr->szmic = szmic;
            mic_length = szmic? TRANSMIC_64BIT : TRANSMIC_32BIT;
        }
        ptr->pdu_base.total_length = pdu_length + mic_length;
        ptr->src = mesh_alloc(pdu_length);
        BX_ASSERT(ptr->src);
        ptr->pdu_base.ctl = 0;
        ptr->pdu_base.seq_auth_valid = 0;
    }
    return ptr;
}

security_credentials_t *access_get_netkey_credentials_by_idx(uint16_t idx,uint8_t credential_flag)
{
    //TODO-FRIEND:
    security_credentials_t* security_credentials = NULL;
    if(credential_flag)
    {

        if(is_lpn_node())
        {
            low_power_get_friend_tx_credentials_by_idx(idx , &security_credentials);
        }else{
            dm_netkey_get_friend_tx_credentials_by_idx(idx , &security_credentials);
        }
        if(security_credentials)
        {
            return security_credentials;
        }
    }
    (void)dm_netkey_get_netkey_tx_credentials_by_idx(idx , &security_credentials);
    BX_ASSERT(security_credentials);
    return security_credentials;
}


access_pdu_tx_t *access_model_msg_build(model_tx_msg_param_t *param,void (*callback)(void *,uint8_t))
{
    uint32_t opcode = param->opcode;
    access_pdu_tx_t *ptr = NULL;
    uint16_t pdu_length = param->pdu_length;

    if(opcode > 0xc00000) {
        pdu_length += 3;
    }
    else if(opcode > 0xFF)
    {
        pdu_length += 2;
    }
    else
    {
        pdu_length += 1;
    }
    ptr = access_tx_pdu_alloc(pdu_length, param->seg, param->szmic);
    if(ptr)
    {
        mesh_core_params_t param_network_transmit;
        mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param_network_transmit);
        if(param->virt)
        {
            BX_ASSERT(param->dst_addr.virt);
            ptr->cryptic.virt_addr = param->dst_addr.virt;
            ptr->pdu_base.dst_addr = param->dst_addr.virt->virt_addr;
        }else
        {
            ptr->cryptic.virt_addr = NULL;
            ptr->pdu_base.dst_addr = param->dst_addr.addr;
        }
        if(param->rx_time)
        {
            uint16_t rand_ms = rand()%(param->virt?VIRT_GROUP_REPLY_DELAY_MAX:UNICART_REPLY_DELAY_MAX - ACCESS_REPLY_DELAY_MIN) + ACCESS_REPLY_DELAY_MIN;
            ptr->pdu_base.expected_tx_time = mesh_time_add(mesh_ms_to_ble_time(rand_ms),*param->rx_time);
            ptr->pdu_base.expected_tx_time_valid = 1;
        }else
        {
            ptr->pdu_base.expected_tx_time_valid = 0;
        }
        if(param->ttl)
        {
            ptr->pdu_base.ttl = *param->ttl;
        }else
        {
            ptr->pdu_base.ttl = config_server_get_default_ttl();
        }
        if(param->interval_ms)
        {
            ptr->pdu_base.interval_ms = *param->interval_ms;
        }else
        {
            ptr->pdu_base.interval_ms = param_network_transmit.network_transmit.interval_steps* 10;
        }
        if(param->repeats)
        {
            ptr->pdu_base.repeats = *param->repeats;
        }else
        {
            ptr->pdu_base.repeats = param_network_transmit.network_transmit.count;
        }
        ptr->pdu_base.src_addr = param->src_addr;
        ptr->pdu_base.callback = callback;
        if(param->akf)
        {
            BX_ASSERT(param->key.app_key);
            ptr->cryptic.appkey = param->key.app_key;
            dm_appkey_get_keybox_info(param->key.app_key,&ptr->cryptic.appkey_box);
            dm_netkey_get_netkey_tx_credentials(param->key.app_key->bound_netkey,&ptr->pdu_base.netkey_credentials);
        }else
        {
            ptr->cryptic.appkey = NULL;
            ptr->cryptic.appkey_box = NULL;
            ptr->pdu_base.netkey_credentials = access_get_netkey_credentials_by_idx(param->key.netkey_idx,param->credential_flag);
        }

        if(opcode > 0xc00000)
        {
            ptr->src[0] = (opcode >> 16) & 0xFF;
            ptr->src[1] = (opcode >> 8) & 0xFF;
            ptr->src[2] = (opcode >> 0) & 0xFF;
        }
        else if(opcode > 0xFF)
        {
            ptr->src[0] = opcode >> 8;
            ptr->src[1] = opcode & 0xFF;
        }
        else
        {
            ptr->src[0] = opcode;
        }
    }
    return ptr;
}

uint8_t *access_model_tx_payload_ptr(access_pdu_tx_t *ptr,uint32_t opcode)
{
    if(opcode > 0xc00000) 
    {
        return ptr->src + 3;
    }else if(opcode > 0xFF)
    {
        return ptr->src + 2;
    }else
    {
        return ptr->src + 1;
    }
}


void access_tx_complete(access_pdu_tx_t *tx,uint8_t status)
{
    mesh_free(tx->encrypted);
    tx->pdu_base.callback(tx,status);
    access_tx_pdu_buf_release(tx);
}

static void access_tx_encrypt_complete()
{
    upper_tx_env_t **ptr = queued_async_process_get_current(&access_tx_async_process);
    upper_tx_env_t *env = *ptr;
    access_pdu_tx_t *pdu = CONTAINER_OF(upper_tx_env_get_current(env),access_pdu_tx_t,pdu_base);
    mesh_free(pdu->src);
    queued_async_process_end(&access_tx_async_process,NULL,0);
}

static void access_tx_pre_process(upper_tx_env_t **ptr)
{
    upper_tx_env_t *env = *ptr;
    access_pdu_tx_t *pdu = CONTAINER_OF(upper_tx_env_get_current(env),access_pdu_tx_t,pdu_base);
    pdu->pdu_base.iv_index = mesh_tx_iv_index_get();
    pdu->encrypted = mesh_alloc(pdu->pdu_base.total_length);
    BX_ASSERT(pdu->encrypted);
    access_pdu_encrypt_start(pdu,access_tx_encrypt_complete);
}

void access_tx_process_start(upper_tx_env_t *ptr)
{
    queued_async_process_start(&access_tx_async_process,&ptr,NULL);
    uint8_t tmp_size = queued_async_process_get_amount(&access_tx_async_process);
    if(tmp_size >  access_max_used_num)
    {
        access_max_used_num = tmp_size;
        LOG(LOG_LVL_INFO,"access tx buf remain %d\n",tmp_size);
    }

}

static void access_tx_process_done(upper_tx_env_t **ptr,void *param,uint8_t status)
{
    LOG(LOG_LVL_INFO,"access tx process done\n");
    upper_tx_setup_start(*ptr);
}

void access_send(access_pdu_tx_t *ptr)
{
    if(provision_is_database_pending())
    {
        ptr->pdu_base.callback(ptr,UPPER_PDU_TX_CANCELED);
        access_tx_pdu_buf_release(ptr);
        return;
    }
    ptr->encrypted = NULL;
    upper_tx_env_add_new_pdu(&ptr->pdu_base);
}

access_pdu_tx_t *access_model_pkt_build_fill(model_tx_msg_param_t *param,void (*callback)(void *,uint8_t),uint8_t *data)
{
    access_pdu_tx_t *ptr = access_model_msg_build(param,callback);
    if(ptr)
    {
        memcpy(access_model_tx_payload_ptr(ptr,param->opcode),data,param->pdu_length);
    }
    return ptr;
}





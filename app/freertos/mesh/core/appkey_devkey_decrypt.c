#include <string.h>
#include "appkey_devkey_decrypt.h"
#include "candidate.h"
#include "aes_ccm_cmac.h"
#include "appkey_devkey_decrypt.h"
#include "static_buffer.h"
#include "app_keys_dm.h"
#include "upper_pdu.h"
#include "co_endian.h"
#include "mesh_env.h"
#include "mesh_queued_msg.h"

static uint8_t nonce[NONCE_LENGTH];
static upper_pdu_rx_t *pdu;
static uint8_t *rslt;
static void (*callback)(uint8_t,app_key_t *,app_key_box_t *);
static virt_addr_mngt_t *virt;
static app_key_t *appkey;
static app_key_box_t *appkey_box;

static void ccm_decrypt_param_build_and_start(uint8_t *key,uint8_t *additional_data,uint8_t additional_data_length,void (*callback)(ccm_cmac_buf_t *,void*,uint8_t))
{
    uint8_t mic_length =  pdu->head.access.szmic ? TRANSMIC_64BIT : TRANSMIC_32BIT;
    ccm_cmac_buf_t ccm_decrypt=
    {
        .param.ccm = {
            .key = key,
            .nonce = nonce,
            .msg = pdu->src,
            .mic_length = mic_length,
            .msg_length = pdu->total_length - mic_length,
            .additional_data = additional_data,
            .additional_data_length = additional_data_length,
            .rslt = rslt,
        },
        .op_type = CCM_DECRYPT,
    };
    ccm_cmac_start(&ccm_decrypt,callback);
}

static void access_pdu_appkey_decrypt_complete(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    if(status == AUTH_FAILED)
    {
        appkey_candidate_remove();
        access_pdu_appkey_decrypt();
    }else
    {
        appkey = appkey_candidate_current_key_env_pick();
        appkey_box = appkey_candidate_current_key_box_pick();
        appkey_candidate_remove_all();
        callback(status,appkey,appkey_box);
    }
}

static void fill_decrypt_app_dev_nonce(uint8_t *nonce,uint8_t nonce_type,upper_pdu_rx_t *pdu)
{
    nonce[0] = nonce_type;
    uint8_t szmic = pdu->head.access.szmic;
    nonce[1] = szmic<<7;
    nonce[2] = pdu->seq_auth >> 16 & 0xff;
    nonce[3] = pdu->seq_auth >> 8 & 0xff;
    nonce[4] = pdu->seq_auth & 0xff;
    nonce[5] = pdu->src_addr >> 8 & 0xff;
    nonce[6] = pdu->src_addr & 0xff;
    nonce[7] = pdu->dst_addr >> 8 & 0xff;
    nonce[8] = pdu->dst_addr & 0xff;
    uint32_t iv_index_big_endian = co_bswap32(pdu->iv_index);
    memcpy(&nonce[9],&iv_index_big_endian,sizeof(uint32_t));
}

static void access_appkey_decrypt_fail(void *param)
{
    callback(AUTH_FAILED,NULL,NULL);
}

void access_pdu_appkey_decrypt(void)
{
    uint8_t *additional_data;
    uint8_t additional_data_length;
    if(virt)
    {
        additional_data = virt->label_uuid;
        additional_data_length = LABEL_UUID_SIZE;
    }else{
        additional_data = NULL;
        additional_data_length = 0;
    }
    app_key_box_t *appkey_box = appkey_candidate_current_key_box_pick();
    if(appkey_box)
    {
        fill_decrypt_app_dev_nonce(nonce,Application_Nonce,pdu);
        ccm_decrypt_param_build_and_start(appkey_box->appkey,additional_data,additional_data_length,access_pdu_appkey_decrypt_complete);
    }else
    {
        mesh_queued_msg_send(access_appkey_decrypt_fail,NULL);
    }
}

void appkey_decrypt_start(upper_pdu_rx_t *upper_pdu,uint8_t *decrypted_buf,virt_addr_mngt_t *virt_addr,void (*decrypt_callback)(uint8_t,app_key_t *,app_key_box_t *))
{
    pdu = upper_pdu;
    rslt = decrypted_buf;
    virt = virt_addr;
    callback = decrypt_callback;
    aid_search(pdu->head.access.aid, pdu->netkey,appkey_candidate_add);
    access_pdu_appkey_decrypt(); 
}

static void access_pdu_devkey_decrypt_complete(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    callback(status,NULL,NULL);
}

void devkey_decrypt_start(upper_pdu_rx_t *upper_pdu,uint8_t *decrypted_buf,void (*decrypt_callback)(uint8_t,app_key_t *,app_key_box_t *))
{
    pdu = upper_pdu;
    rslt = decrypted_buf;
    callback = decrypt_callback;    
    uint8_t *devkey = get_peer_devkey(pdu->src_addr,pdu->dst_addr);
    fill_decrypt_app_dev_nonce(nonce,Device_Nonce,pdu);
    ccm_decrypt_param_build_and_start(devkey,NULL,0,access_pdu_devkey_decrypt_complete);
}

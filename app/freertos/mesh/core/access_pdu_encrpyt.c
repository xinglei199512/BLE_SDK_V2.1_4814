#include "access_pdu_encrypt.h"
#include "aes_ccm_cmac.h"
#include "mesh_env.h"
#include "co_endian.h"
#include <string.h>
#include "upper_pdu.h"

static uint8_t nonce[NONCE_LENGTH];


static void fill_encrypt_app_dev_nonce(uint8_t *nonce,uint8_t nonce_type,access_pdu_tx_t *pdu)
{
    nonce[0] = nonce_type;
    uint8_t szmic = pdu->szmic;
    nonce[1] = szmic<<7;
    nonce[2] = pdu->pdu_base.seq_auth >> 16 & 0xff;
    nonce[3] = pdu->pdu_base.seq_auth >> 8 & 0xff;
    nonce[4] = pdu->pdu_base.seq_auth & 0xff;
    nonce[5] = pdu->pdu_base.src_addr >> 8 & 0xff;
    nonce[6] = pdu->pdu_base.src_addr & 0xff;
    nonce[7] = pdu->pdu_base.dst_addr >> 8 & 0xff;
    nonce[8] = pdu->pdu_base.dst_addr & 0xff;
    uint32_t iv_index_big_endian = co_bswap32( pdu->pdu_base.iv_index);
    memcpy(&nonce[9],&iv_index_big_endian,sizeof(uint32_t));
}



static void ccm_encrypt_param_build_and_start(access_pdu_tx_t *pdu,uint8_t *key,
    uint8_t *additional_data,uint8_t additional_data_length,uint8_t *rslt,void (*callback)(ccm_cmac_buf_t*,void*,uint8_t))
{
    uint8_t mic_length =  pdu->szmic ? TRANSMIC_64BIT : TRANSMIC_32BIT;
    ccm_cmac_buf_t ccm_encrypt=
    {
        .param.ccm = {
            .key = key,
            .nonce = nonce,
            .msg = pdu->src,
            .mic_length = mic_length,
            .msg_length = pdu->pdu_base.total_length - mic_length,
            .additional_data = additional_data,
            .additional_data_length = additional_data_length,
            .rslt = rslt,
        },
        .op_type = CCM_ENCRYPT,
    };
    ccm_cmac_start(&ccm_encrypt,callback);
}

void access_pdu_encrypt_start(access_pdu_tx_t *pdu,void (*callback)())
{
    uint8_t *key;
    if(pdu->cryptic.appkey)
    {
        //TODO fill app nonce
        fill_encrypt_app_dev_nonce(nonce,Application_Nonce,pdu);
        key =(uint8_t *) pdu->cryptic.appkey_box->appkey;
    }else
    {
        //TODO fill dev nonce
        fill_encrypt_app_dev_nonce(nonce,Device_Nonce,pdu);
        key = get_devkey(pdu->pdu_base.src_addr,pdu->pdu_base.dst_addr);
    }
    uint8_t *additional_data;
    uint8_t additional_data_length;
    if(pdu->cryptic.virt_addr)
    {
        additional_data = pdu->cryptic.virt_addr->label_uuid;
        additional_data_length = LABEL_UUID_SIZE;
    }else
    {
        additional_data = NULL;
        additional_data_length = 0;
    }
    ccm_encrypt_param_build_and_start(pdu,key,additional_data,additional_data_length,pdu->encrypted,(void (*)(ccm_cmac_buf_t*,void*,uint8_t))callback);
}





#include "network_pdu_encrypt.h"
#include "queued_async_framework.h"
#include "adv_bearer_tx.h"
#include "network_pdu.h"
#include "aes_ccm_cmac.h"
#include "obfuscation.h"
#include "co_endian.h"
#include "string.h"

static uint8_t nonce[NONCE_LENGTH];
static struct{
    network_tx_data_t *src;
    uint32_t iv_index;
    security_credentials_t *security;
    void (*cb)(network_pdu_packet_u *);
    network_pdu_packet_u *rslt;
    uint8_t pkt_type;
}network_encrypt_env;

static void tx_obfuscation_complete(obfuscation_param_t *obfuscation_base,void *dummy,uint8_t status)
{
    network_encrypt_env.cb(network_encrypt_env.rslt);
}

static void network_pdu_ccm_encrypt_complete(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    network_pdu_packet_u *pkt_u = (network_pdu_packet_u *)network_encrypt_env.rslt->buf;
    obfuscation_param_t buf = {
            .privacy_key = network_encrypt_env.security->privacy_key,
            .privacy_random = (uint8_t *)&pkt_u->pkt.dst_be,
            .iv_index = network_encrypt_env.iv_index,
            .src_data = (uint8_t *)&network_encrypt_env.src->head + 1,
            .rslt = &network_encrypt_env.rslt->buf[1],
    };
    obfuscation_start(&buf,tx_obfuscation_complete);
}

static void fill_network_nonce(uint8_t *nonce,uint8_t *octet1_6,uint32_t iv_index)
{
    nonce[0] = Network_Nonce;
    memcpy(&nonce[1],octet1_6,OBFUSCATED_DATA_LENGTH);
    nonce[7] = 0;
    nonce[8] = 0;
    iv_index = co_bswap32(iv_index);
    memcpy(&nonce[9],&iv_index,sizeof(uint32_t));
}


static void fill_proxy_nonce(uint8_t *nonce,uint8_t *octet2_6,uint32_t iv_index)
{
    nonce[0] = Proxy_Nonce;//type
    nonce[1] = 0x00;//pad
    memcpy(&nonce[2],octet2_6,5);//seq+src = 3+2 =5
    nonce[7] = 0;//pad
    nonce[8] = 0;//pad
    iv_index = co_bswap32(iv_index);
    memcpy(&nonce[9],&iv_index,sizeof(uint32_t));
}



static void network_pdu_ccm_encrypt()
{
    if(network_encrypt_env.pkt_type == NON_ADV_PROXY_CONFIG_PKT)
    {
        //fill proxy nonce
        fill_proxy_nonce(nonce,(uint8_t *)&network_encrypt_env.src->head + 2,network_encrypt_env.iv_index);
    }else
    {
        //fill network nonce
        fill_network_nonce(nonce,(uint8_t *)&network_encrypt_env.src->head + 1,network_encrypt_env.iv_index);
    }
    network_pdu_packet_u *pkt_u = (network_pdu_packet_u *)&network_encrypt_env.src->head;
    uint8_t mic_length = pkt_u->pkt.ctl ? CONTROL_MSG_NETMIC_SIZE : ACCESS_MSG_NETMIC_SIZE;
    uint8_t msg_length = network_encrypt_env.src->lower_pdu_length + 2 ;
    ccm_cmac_buf_t buf={
        .param.ccm = 
        {
            .key = network_encrypt_env.security->encryption_key,
            .nonce = nonce,
            .msg = (uint8_t *)&pkt_u->pkt.dst_be,
            .additional_data = NULL,
            .additional_data_length = 0,
            .msg_length = msg_length,
            .mic_length = mic_length,
            .rslt = &network_encrypt_env.rslt->buf[ENCRYPTED_DATA_OFFSET],
        },
        .op_type = CCM_ENCRYPT,
    };
    ccm_cmac_start(&buf,network_pdu_ccm_encrypt_complete);
}

void network_pdu_encrypt_start(network_tx_data_t *src,uint32_t iv_index,security_credentials_t *security,
    void (*callback)(network_pdu_packet_u *),uint8_t pkt_type,network_pdu_packet_u *rslt_buf)
{
    network_encrypt_env.src = src;
    network_encrypt_env.iv_index = iv_index;
    network_encrypt_env.security = security;
    network_encrypt_env.cb = callback;
    network_encrypt_env.rslt = rslt_buf;
    network_encrypt_env.pkt_type = pkt_type;
    network_pdu_ccm_encrypt();
}




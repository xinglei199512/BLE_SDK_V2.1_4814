#define LOG_TAG        "network_pdu_decrypt.c"
#define LOG_LVL        LVL_DBG
#include "bx_log.h"


#include <string.h>
#include "network_pdu_decrypt.h"
#include "obfuscation.h"
#include "aes_ccm_cmac.h"
#include "candidate.h"
#include "network_pdu.h"
#include "mesh_iv_operation_ex.h"
#include "network_keys_dm.h"
#include "co_endian.h"
#include "mesh_queued_msg.h"

static uint8_t nonce[NONCE_LENGTH];
static network_pdu_decrypt_callback_param_t decrypt_rslt;
static mesh_adv_data_t *input_src;
static void (*decrypt_cb)(uint8_t,network_pdu_decrypt_callback_param_t *);
static uint8_t network_pdu_type;
static void decryption_procedure_trigger(void);

static void netkey_decrypt_fail()
{
    netkey_candidate_remove();
    decryption_procedure_trigger();
}

static void ccm_decrypt_complete(ccm_cmac_buf_t *ccm_base,void *dummy,uint8_t status)
{
    if(status == AUTH_FAILED)
    {
        netkey_decrypt_fail();
    }else
    {
        decrypt_rslt.netkey = netkey_candidate_current_key_env_pick();
        decrypt_rslt.net_security = netkey_candidate_current_key_box_pick();
        netkey_candidate_remove_all();
        decrypt_rslt.decrypted.buf[0] = input_src->data[0];
        decrypt_cb(status,&decrypt_rslt);
    }
}

bool proxy_or_friend_network_pdu_check(network_pdu_packet_u const*pkt)
{
    return pkt->pkt.ctl == 1 && pkt->pkt.ttl == 0;
}

uint8_t network_pdu_mic_length(uint8_t ctl)
{
    return ctl ? CONTROL_MSG_NETMIC_SIZE : ACCESS_MSG_NETMIC_SIZE;
}

uint8_t network_pdu_encrypt_msg_length(uint8_t total_length,uint8_t ctl)
{
    uint8_t mic_length = network_pdu_mic_length(ctl);
    uint8_t msg_length = total_length - ENCRYPTED_DATA_OFFSET - mic_length;
    return msg_length;
}

uint8_t network_transport_pdu_length(uint8_t total_length,uint8_t ctl)
{
    return network_pdu_encrypt_msg_length(total_length,ctl) - 2;
}

static void fill_proxy_nonce(uint8_t *nonce,const uint8_t *p_octet2_6,uint32_t iv_index)
{
    nonce[0] = Proxy_Nonce;//type
    nonce[1] = 0x00;//pad
    memcpy(&nonce[2],p_octet2_6,5);//seq+src = 3+2 =5
    nonce[7] = 0;//pad
    nonce[8] = 0;//pad
    iv_index = co_bswap32(iv_index);
    memcpy(&nonce[9],&iv_index,sizeof(uint32_t));
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

static void de_obfuscation_complete(obfuscation_param_t *obfuscation_base,void *dummy,uint8_t status)
{
    bool proxy_or_friend = proxy_or_friend_network_pdu_check(&decrypt_rslt.decrypted);
    uint32_t iv_index = decrypt_rslt.iv_index;
    if(network_pdu_type == PROXY_CONFIG_NETWORK_PKT_RX && proxy_or_friend == true)
    {
        fill_proxy_nonce(nonce,&decrypt_rslt.decrypted.buf[2],iv_index);
        //fill proxy nonce
    }else if(network_pdu_type == NORMAL_NETWORK_PKT_RX)
    {
        //fill network nonce
        fill_network_nonce(nonce,&decrypt_rslt.decrypted.buf[1],iv_index);
    }else
    {
        netkey_decrypt_fail();
        return;
    }

    uint8_t mic_length = network_pdu_mic_length(decrypt_rslt.decrypted.pkt.ctl);
    uint8_t msg_length = network_pdu_encrypt_msg_length(input_src->length,decrypt_rslt.decrypted.pkt.ctl);
    security_credentials_t *net_security = netkey_candidate_current_key_box_pick();
    network_pdu_packet_u *pkt_u = (network_pdu_packet_u *)input_src->data;
    ccm_cmac_buf_t buf = 
    {
         .param.ccm = 
        {
            .key = net_security->encryption_key,
            .nonce = nonce,
            .msg = (uint8_t *)&pkt_u->pkt.dst_be,
            .additional_data = NULL,
            .additional_data_length = 0,
            .msg_length = msg_length,
            .mic_length = mic_length,
            .rslt = &decrypt_rslt.decrypted.buf[ENCRYPTED_DATA_OFFSET],
        },
        .op_type = CCM_DECRYPT,
    };
    ccm_cmac_start(&buf,ccm_decrypt_complete);
}


static void obfuscation_param_build_and_start(security_credentials_t *net_security)
{
    network_pdu_packet_u *pkt_u = (network_pdu_packet_u *)input_src->data;
    decrypt_rslt.iv_index = mesh_rx_iv_index_get(pkt_u->pkt.ivi);
    obfuscation_param_t buf=
    {
            .privacy_key = net_security->privacy_key,
            .privacy_random = (uint8_t *)&pkt_u->pkt.dst_be,
            .iv_index = decrypt_rslt.iv_index,
            .src_data = &pkt_u->buf[1],
            .rslt = &decrypt_rslt.decrypted.buf[1],
     };
    obfuscation_start(&buf,de_obfuscation_complete);
}

static void network_decrypt_fail(void *param)
{
    decrypt_cb(AUTH_FAILED,NULL);
}

static void decryption_procedure_trigger(void)
{
    security_credentials_t *net_security = netkey_candidate_current_key_box_pick();
    if(net_security)
    {
        obfuscation_param_build_and_start(net_security);
    }else
    {
        mesh_queued_msg_send(network_decrypt_fail,NULL);
    }
}

void network_pdu_decrypt_start(mesh_adv_data_t *src,uint8_t pdu_type,void (*callback)(uint8_t,network_pdu_decrypt_callback_param_t *))
{
    input_src = src;
    network_pdu_type = pdu_type;
    decrypt_cb = callback;
    network_pdu_packet_head_t *pkt = (network_pdu_packet_head_t *)input_src->data;
    //LOG_D("%s,nid=0x%x",__func__,pkt->nid);
    dm_netkey_nid_search(pkt->nid,netkey_candidate_add);
    decryption_procedure_trigger();
}

uint16_t network_pdu_get_src_addr(network_pdu_decrypt_callback_param_t * p_pdu)
{
    return co_bswap16(p_pdu->decrypted.pkt.src_be);
}

uint16_t network_pdu_get_dst_addr(network_pdu_decrypt_callback_param_t * p_pdu)
{
    return co_bswap16(p_pdu->decrypted.pkt.dst_be);
}

uint8_t * network_pdu_get_transport_pdu(network_pdu_decrypt_callback_param_t * p_pdu)
{
    return p_pdu->decrypted.buf+sizeof(network_pdu_packet_head_t);
}


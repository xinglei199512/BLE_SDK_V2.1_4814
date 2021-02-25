#ifndef NETWORK_PDU_DECRYPT_H_
#define NETWORK_PDU_DECRYPT_H_
#include "network_pdu.h"


typedef struct
{
    network_pdu_packet_u decrypted;
    net_key_t *netkey;
    security_credentials_t *net_security;
    uint32_t iv_index;
}network_pdu_decrypt_callback_param_t;

void network_pdu_decrypt_start(mesh_adv_data_t *src,uint8_t pdu_type,void (*callback)(uint8_t,network_pdu_decrypt_callback_param_t *));
uint16_t network_pdu_get_src_addr(network_pdu_decrypt_callback_param_t * p_pdu);
uint8_t network_pdu_encrypt_msg_length(uint8_t total_length,uint8_t ctl);
uint8_t network_transport_pdu_length(uint8_t total_length,uint8_t ctl);
uint16_t network_pdu_get_dst_addr(network_pdu_decrypt_callback_param_t * p_pdu);
uint8_t * network_pdu_get_transport_pdu(network_pdu_decrypt_callback_param_t * p_pdu);
uint8_t network_pdu_mic_length(uint8_t ctl);



#endif
 

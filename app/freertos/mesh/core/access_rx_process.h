#ifndef ACCESS_RX_PROCESS_H_
#define ACCESS_RX_PROCESS_H_
#include "upper_pdu.h"
#include "access_pdu_decrypt.h"

#define ACCESS_DEFAULT_TTL                      10


typedef struct
{
    upper_pdu_rx_t* uppdu;
    void * access_payload;
    access_pdu_decrypt_callback_param_t* param;
}access_pdu_rx_t;

ble_txrx_time_t access_rx_get_rx_time(access_pdu_rx_t *ptr);
uint16_t get_access_pdu_rx_payload_size(access_pdu_rx_t *pdu);
uint16_t access_get_pdu_src_addr(access_pdu_rx_t * access_pdu);
uint16_t access_get_pdu_dst_addr(access_pdu_rx_t * access_pdu);
virt_addr_mngt_t *  access_get_pdu_virt_addr(access_pdu_rx_t * access_pdu);
uint8_t * access_get_pdu_payload(access_pdu_rx_t * access_pdu);
uint16_t access_get_pdu_payload_length(access_pdu_rx_t * access_pdu);
uint16_t access_get_pdu_mic_length(access_pdu_rx_t * access_pdu);
uint8_t  access_get_pdu_op_head(access_pdu_rx_t * access_pdu);
lower_pdu_head_info_u*  access_get_pdu_type_header(access_pdu_rx_t * access_pdu);
uint8_t  access_is_access_pdu_type(access_pdu_rx_t * access_pdu);
app_key_box_t*  access_get_pdu_app_keybox(access_pdu_rx_t * access_pdu);
app_key_t*  access_get_pdu_app_key(access_pdu_rx_t * access_pdu);
uint16_t  access_get_pdu_netkey_global_index(access_pdu_rx_t * access_pdu);
uint16_t  access_get_pdu_appkey_global_index(access_pdu_rx_t * access_pdu);

void access_rx_process_start(upper_pdu_rx_t *buf);

#endif

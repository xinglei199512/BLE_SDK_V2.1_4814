#ifndef UPPER_PDU_H_
#define UPPER_PDU_H_
#include <stdint.h>
#include "co_list.h"
//#include "adv_bearer.h"
#include "sdk_mesh_definitions.h"
#include "mesh_ble_time.h"

#define TRANSMIC_64BIT                      8
#define TRANSMIC_32BIT                      4

enum control_opcode
{
    FRIEND_POLL = 0x01,
    FRIEND_UPDATE,
    FRIEND_REQUEST,
    FRIEND_OFFER,
    FRIEND_CLEAR,
    FRIEND_CLEAR_CONFIRM,
    FRIEND_SUBSCRIPTION_LIST_ADD,
    FRIEND_SUBSCRIPTION_LIST_REMOVE,
    FRIEND_SUBSCRIPTION_LIST_CONFIRM,
    HEARTBEAT,
    CONTROL_PDU_TYPE_MAX,
};

enum upper_pdu_tx_stat
{
    UPPER_TX_SUCCESS,
    UPPER_TX_CANCEL,
};

#define SEQZERO_LENGTH                      13
#define SEGO_LENGTH                         5
#define SEGN_LENGTH                         5
#define SEGMENTED_ACCESS_MSG_MAX_LENGTH     12
#define SEGMENTED_CONTROL_MSG_MAX_LENGTH    8
#define TRANSPORT_PDU_OFFSET                9


typedef struct
{
    uint8_t aid:6,
            akf:1,
            seg:1;
}access_pkt_head_t;

typedef struct
{
    uint8_t opcode:7,
                    seg:1;
}control_pkt_head_t;

typedef struct
{
    uint8_t head:7,
            seg:1;
}lower_pkt_head_t;

typedef union
{
    access_pkt_head_t access;
    control_pkt_head_t control;
    lower_pkt_head_t head;
}lower_pkt_head_u;

typedef struct
{
    uint8_t aid:6,
            akf:1,
            szmic:1;   
}access_pdu_info_t;

typedef struct
{
    uint8_t opcode:7;
}control_pdu_info_t;

typedef union{
    access_pdu_info_t access;
    control_pdu_info_t control;
}lower_pdu_head_info_u;

typedef struct
{
    ble_txrx_time_t rx_time;
    uint8_t *src;
    uint32_t iv_index;
    uint32_t seq_auth:24;
    net_key_t *netkey;
    security_credentials_t *net_security;
    uint16_t total_length;
    uint16_t src_addr;
    uint16_t dst_addr;
    lower_pdu_head_info_u head;
    uint8_t from;
    uint8_t rssi;
    uint8_t ttl:7,
            ctl:1;
}upper_pdu_rx_t;


typedef struct
{
    app_key_t *appkey;
    app_key_box_t *appkey_box;
    virt_addr_mngt_t *virt_addr;
}access_pdu_cryptic_param_t;


typedef struct upper_pdu_tx_base_s
{
    struct co_list_hdr hdr;
    void (*callback)(void *,uint8_t);
    security_credentials_t *netkey_credentials;
    uint32_t iv_index;
    ble_txrx_time_t expected_tx_time;
    uint32_t seq_auth:24,
                    seq_auth_valid:1,
                    expected_tx_time_valid:1,
                    seg:1,
                    low_power_mode:1;
    uint16_t interval_ms;
    uint16_t total_length;
    uint16_t src_addr;
    uint16_t dst_addr;
    uint8_t ttl:7,
                   ctl:1;
    uint8_t repeats;
}upper_pdu_tx_base_t;

typedef struct
{
    uint8_t *src;
    upper_pdu_tx_base_t pdu_base;
    access_pdu_cryptic_param_t cryptic;
    uint8_t *encrypted;
    uint8_t szmic:1;
}access_pdu_tx_t;

typedef struct
{
    uint8_t *payload;
    upper_pdu_tx_base_t pdu_base;
    uint8_t opcode:7,
                    high_priority:1;
}control_pdu_tx_t;


#endif

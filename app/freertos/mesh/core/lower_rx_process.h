#ifndef LOWER_PROCESS_H_
#define LOWER_PROCESS_H_

#include "network_pdu_decrypt.h"
#include "timer_wrapper.h"
#include "friend.h"




#define SEQZERO_MASK                                0x7ffc00
#define SEQZERO_OFFSET                              10
#define SEGN_MASK                                   0x1f
#define SEGN_OFFSET                                 0
#define SEGO_MASK                                   0x3e0
#define SEGO_OFFSET                                 5
#define SEGMENT_ACK_DEFAULT_TTL                     10
#define SEGMENTED_PAYLOAD_OFFSET                    4
#define ACK_TIMER(TTL)                              (150 + 50*(TTL))

typedef struct
{
    mesh_timer_t incomplete;
    mesh_timer_t acknowledgment;    
}reassembly_timer_t;

typedef struct
{
    uint8_t *data;
    net_key_t *netkey;
    security_credentials_t *net_security;
}local_reassembly_t;

typedef struct
{
    reassembly_timer_t timer;
    friend_list_t *friend;
    local_reassembly_t *local;
    uint32_t block_ack;
    uint16_t dst_addr;
    uint8_t last_seg_length;
}reassembly_env_t;

enum lower_rx_env_stat
{
    SEG_PDU_RX,
    COMPLETE_PDU_RX,
    INCOMPLETE_TIMEOUT,
};

typedef struct
{
    struct co_list_hdr hdr;
    reassembly_env_t *reassembly;
    uint32_t iv_index;
    uint32_t current_seq_auth:24;
    uint16_t src_addr;
    enum lower_rx_env_stat state;
}lower_rx_env_t;


void incomplete_timer_callback(mesh_timer_t timer);
void ack_timer_callback(mesh_timer_t timer);
void lower_transport_rx(network_pdu_decrypt_callback_param_t *param,uint8_t total_length,uint8_t from,ble_txrx_time_t rx_time,uint8_t rssi,friend_list_t *friend,bool local);
uint16_t lower_pdu_seq_zero_get(uint8_t *lower_pdu);
void lower_rx_env_reassembly_cleanup(void);



#endif

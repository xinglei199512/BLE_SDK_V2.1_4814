#ifndef NETWORK_PDU_H_
#define NETWORK_PDU_H_
#include <stdint.h>
#include <stdbool.h>
#include "gap.h"
#include "mesh_model.h"
#include "upper_pdu.h"
#include "timer_wrapper.h"
#define ADV_DATA_BUF_SIZE (GAP_ADV_DATA_LEN)
#define BEARER_BUF_SIZE (ADV_DATA_BUF_SIZE - 2)

#define ADV_BEARER_RX (0xfe)
#define LOCAL_NETWORK_INTERFACE_RX (0xff)


#define ACCESS_MSG_NETMIC_SIZE                  4
#define CONTROL_MSG_NETMIC_SIZE                 8
#define ENCRYPTED_DATA_OFFSET       7
#define TRANSPORT_DATA_OFFSET       9

enum network_pkt_type_rx
{
    NORMAL_NETWORK_PKT_RX,
    PROXY_CONFIG_NETWORK_PKT_RX,
};

typedef struct __attribute__((packed))
{
    uint8_t nid:7,
            ivi:1;
    uint8_t ttl:7,
            ctl:1;
    uint32_t seq_be:24;
    uint16_t src_be;
    uint16_t dst_be;
}network_pdu_packet_head_t;

typedef union __attribute__((packed)){
    network_pdu_packet_head_t pkt;
    uint8_t buf[BEARER_BUF_SIZE];
}network_pdu_packet_u;

typedef struct
{
    uint8_t data[BEARER_BUF_SIZE];
    uint8_t length;
}mesh_adv_data_t;

typedef struct
{
    ble_txrx_time_t rx_time;
    mesh_adv_data_t src;
    uint8_t type;
    uint8_t from;
    uint8_t rssi;
}network_pdu_rx_t;

typedef struct __attribute__((packed))
{
    network_pdu_packet_head_t head;
    uint8_t lower_pdu[BEARER_BUF_SIZE - sizeof(network_pdu_packet_head_t) - TRANSMIC_32BIT];
    uint8_t lower_pdu_length;
}network_tx_data_t;

typedef struct
{
    uint32_t iv_index;
    security_credentials_t *netkey_credentials;
    network_tx_data_t src;
    uint8_t pkt_type;
} network_pdu_tx_t;

typedef struct
{
    net_key_t *netkey;
    security_credentials_t *net_security;
    uint32_t block_ack;
    uint32_t seq_zero:13,
                ttl:7,
                obo:1;
    uint16_t src_addr;
    uint16_t dst_addr;
}segment_ack_param_t;

#define THREE_BYTES_ENDIAN_REVERSE(x) (((x)>>16)&0xff)|((x)&0xff00)|(((x)<<16)&0xff0000)
#define SEQ_ENDIAN_REVERSE(x) THREE_BYTES_ENDIAN_REVERSE(x)

#endif

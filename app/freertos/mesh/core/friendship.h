#ifndef FRIENDSHIP_H_
#define FRIENDSHIP_H_
#include <stdint.h>
#include "co_list.h"
#include "sdk_mesh_definitions.h"


typedef struct
{
    struct co_list_hdr hdr; 
    net_key_t *netkey;
    security_credentials_t credentials[2]; 
}friend_security_t;

typedef struct __attribute__((packed))
{
    uint8_t min_queue_size_log:3,
            receive_window_factor:2,
            rssi_factor:2;
}friend_req_criteria_t;

typedef struct __attribute__((packed))
{
    uint8_t fsn:1,
            padding:7;
}friend_poll_payload_t;

typedef struct __attribute__((packed))
{
    friend_req_criteria_t criteria;
    uint8_t receive_delay;
    uint32_t poll_timeout_be:24;
    uint16_t prev_addr_be;
    uint8_t elem_nums;
    uint16_t lpn_counter_be;
}friend_request_payload_t;

typedef struct __attribute__((packed))
{
    uint8_t receive_window;
    uint8_t queue_size;
    uint8_t subscription_list_size;
    uint8_t rssi;
    uint16_t friend_counter_be;
}friend_offer_payload_t;

typedef struct __attribute__((packed))
{
    uint8_t key_refresh_phase_2:1,
                    iv_update_active:1,
                    reserved:6;
}friend_update_flag_t;

typedef struct __attribute__((packed))
{
    friend_update_flag_t flags;
    uint32_t iv_index_be;
    uint8_t md;
}friend_update_payload_t;

typedef struct __attribute__((packed))
{
    uint16_t lpn_addr_be;
    uint16_t lpn_counter_be;
}friend_clear_payload_t;

typedef struct __attribute__((packed))
{
    uint16_t lpn_addr_be;
    uint16_t lpn_counter_be;
}friend_clear_confirm_payload_t;

typedef struct __attribute__((packed))
{
    uint8_t transaction_number;
    uint16_t address_list_be[__ARRAY_EMPTY];
}friend_sunscription_list_add_payload_t;

typedef struct __attribute__((packed))
{
    uint8_t transaction_number;
    uint16_t address_list_be[__ARRAY_EMPTY];
}friend_sunscription_list_remove_payload_t;

typedef struct __attribute__((packed))
{
    uint8_t transaction_number;
}friend_sunscription_list_confirm_payload_t;



void friend_clear_tx(uint16_t dst_addr,uint8_t ttl,security_credentials_t *net_security,uint16_t lpn_addr,uint16_t lpn_counter,void (*callback)(void *,uint8_t));


#endif

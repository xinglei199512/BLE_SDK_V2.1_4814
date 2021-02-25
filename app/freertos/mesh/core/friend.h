#ifndef FRIEND_H_
#define FRIEND_H_
#include "co_list.h"
#include "network_pdu_decrypt.h"
#include "static_buffer.h"
#include "timer_wrapper.h"
#include "adv_bearer_tx.h"
#include "friendship.h"
#include "stack_mem_cfg.h"




typedef struct
{
    struct co_list_hdr hdr;
    network_pdu_tx_t *tx_pdu;
    network_adv_tx_t *tx_adv;
}friend_q_element_t;

DECLARE_ARRAY_BUF_TYPE(friend_q_t,friend_q_element_t,QUEUE_LENGTH_PER_FRIEND);

typedef struct
{
    friend_q_t queue;
    struct co_list seg_list;
    struct co_list msg_list;
    ble_txrx_time_t expected_tx_time;
    friend_q_element_t *current_tx;
    friend_security_t security;
    mesh_timer_t poll_timer;
    mesh_timer_t clear_repeat_timer;
    mesh_timer_t clear_procedure_timer;
    uint16_t poll_timeout;
    uint16_t lpn_counter;
    uint16_t subscript_list[SUBSCRIPTION_LIST_SIZE_PER_FRIEND];
    uint16_t uni_addr;
    uint16_t prev_friend;
    uint8_t element_nums;
    uint8_t receive_delay;
    uint8_t fsn:1;
    bool friendship_established;
    uint16_t friend_counter;
}friend_list_t;


uint16_t friend_q_available_size(friend_q_t *ptr);
friend_q_element_t *friend_q_element_alloc(friend_q_t *ptr);
void friend_q_element_release(friend_q_t *ptr,friend_q_element_t *element);
void friend_q_element_env_free(friend_q_t *ptr,friend_q_element_t *element);
friend_q_element_t *friend_q_seg_ack_search(friend_list_t *ptr,uint16_t src_addr,uint16_t dst_addr,uint16_t seq_zero);
bool friend_q_remove(friend_list_t *ptr,friend_q_element_t *element);
friend_q_element_t *friend_q_remove_oldest_entry(friend_list_t *ptr);
friend_list_t *friend_env_calloc(void);
void friend_env_retain(friend_list_t *env);
void friend_env_release(friend_list_t *env);
friend_list_t *friend_search_for_caching_msg(uint16_t dst_addr);
friend_list_t *friend_primary_addr_search(uint16_t primary_addr);
void alloc_friend_q_element_and_push_to_list(friend_list_t *env ,network_pdu_packet_u *data,uint8_t total_length,struct co_list *list);
void friend_q_pop_and_send(friend_list_t *ptr,ble_txrx_time_t expected_tx_time);
uint32_t friend_tx_delay_calc(ble_txrx_time_t expected_tx_time);
uint32_t friend_queue_pkt_tx_start(uint16_t dst_addr,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx);
uint16_t friend_get_cached_msg_num(friend_list_t *env);
void friend_update_add_to_q(friend_list_t *ptr,bool key_refresh_phase_2,bool iv_update_active);
void friend_poll_rx_handler(upper_pdu_rx_t *ptr);
void friend_offer_tx(uint16_t dst_addr,uint8_t rssi,ble_txrx_time_t expected_tx_time,security_credentials_t *net_security);
void friend_queue_list_cleanup(friend_list_t *env,struct co_list *list);
void friend_request_rx_handler(upper_pdu_rx_t *ptr);
void friend_clear_rx_handler(upper_pdu_rx_t *ptr);
void friend_clear_confirm_rx_handler(upper_pdu_rx_t *ptr);
void friend_subscription_list_add_rx_handler(upper_pdu_rx_t *ptr);
void friend_subscription_list_remove_rx_handler(upper_pdu_rx_t *ptr);
void friend_seg_list_cleanup(friend_list_t *env);
void friend_update_add_to_q_for_all(uint8_t* network_id);
void generate_friend_credentials(void* dm_handle);



#endif

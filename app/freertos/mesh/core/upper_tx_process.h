#ifndef UPPER_TX_PROCESS_H_
#define UPPER_TX_PROCESS_H_
#include <stdbool.h>
#include <stdint.h>
#include "upper_pdu.h"
#include "co_list.h"
#include "adv_bearer_tx.h"
#include "timer_wrapper.h"

#define SEG_TX_TIMER(TTL)                           (200 + 50 *(TTL))


enum upper_tx_stat_enum
{
    UPPER_PDU_TX_COMPLETE,
    UPPER_PDU_TX_CANCELED,
};

typedef struct
{
    network_adv_tx_t *adv;
    uint8_t next_state;
}upper_adv_tx_env_t;

typedef struct
{
    upper_adv_tx_env_t *adv_tx;
    mesh_timer_t segment_tx_timer;
    uint32_t block_ack;
    uint8_t SegN;
    uint8_t tx_cnt;
    uint8_t SegO_to_build;
    bool ack_received;
    bool need_resched;
}upper_tx_mngt_t;

typedef struct
{
    struct co_list tx_list;
    upper_tx_mngt_t *mngt;
}upper_tx_env_t;

void upper_tx_env_add_new_pdu(upper_pdu_tx_base_t *pdu_base);

void segment_ack_rx(uint16_t addr,uint16_t seq_zero,uint32_t block_ack);

void upper_tx_setup_start(upper_tx_env_t *env);

uint32_t upper_pkt_tx_start(uint16_t dst_addr,uint8_t ctl,bool *local,bool *adv,uint32_t *gatt_mask,network_adv_tx_t *adv_tx);

bool upper_adv_tx_start_hook(network_adv_tx_t *ptr,bool adv_cnt_expire,uint16_t *interval_ms);


bool upper_tx_env_adv_tx_timer_expire_handler(network_adv_tx_t *ptr);

upper_pdu_tx_base_t *upper_tx_env_get_current(upper_tx_env_t *env);

#endif

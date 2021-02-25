#ifndef ADV_BEARER_TX_H_
#define ADV_BEARER_TX_H_
#include <stdint.h>
#include <stdbool.h>
#include "gap.h"
#include "timer_wrapper.h"
#include "network_pdu.h"
#define DATA_OFFSET_IN_BEARER                   2


enum pkt_type_tx
{
    PROVISION_BEARER_ADV_PKT,
    UNPROV_DEV_BEACON_ADV_PKT,
    SECURE_NETWORK_BEACON_ADV_PKT,
    RELAY_ADV_PKT,
    SEGMENT_ACK_ADV_PKT,
    FRIEND_QUEUE_ADV_PKT,
    UPPER_LAYER_ADV_PKT,
    GATT_SERVICE_ADV_PKT,
    NON_ADV_PROXY_CONFIG_PKT,
};

typedef struct
{
    uint8_t count;
    uint8_t repeats;
    bool high_priority;
    bool delayed_start;
}network_tx_param_t;

typedef struct
{
    uint8_t count;
    uint8_t repeats;
}pb_tx_param_t;

typedef struct
{
    uint8_t count;
    uint8_t interval_steps;
    uint8_t repeats;
}beacon_tx_param_t;


typedef struct
{
    uint8_t length;
    uint8_t data[ADV_DATA_BUF_SIZE];
}mesh_adv_scan_rsp_t;

typedef struct
{
    mesh_timer_t timer;
    mesh_adv_scan_rsp_t *scan_rsp;
    uint8_t pkt_type;
    uint8_t data_length;
    uint8_t adv_data[ADV_DATA_BUF_SIZE];
}mesh_adv_tx_t;

typedef struct
{
    mesh_adv_tx_t adv;
    uint16_t dst_addr;
    uint8_t ctl;
    network_tx_param_t param;
}network_adv_tx_t;

typedef struct
{
    mesh_adv_tx_t adv;
    beacon_tx_param_t param;
}beacon_adv_tx_t;


typedef struct
{
    mesh_adv_tx_t adv;
    pb_tx_param_t param;
}pb_adv_tx_t;


void bearer_adv_tx_msg_send(mesh_adv_tx_t *ptr,bool high_priority);
void beacon_adv_tx_buf_free(beacon_adv_tx_t *ptr);
beacon_adv_tx_t *beacon_adv_pkt_prepare(uint8_t *data,uint8_t length,uint8_t pkt_type);
beacon_adv_tx_t *beacon_adv_tx_buf_alloc(uint8_t pkt_type);
pb_adv_tx_t *pb_adv_pkt_prepare(uint8_t *data,uint8_t length,uint8_t pkt_type);
void bearer_adv_tx_timer_start(mesh_adv_tx_t *adv,uint16_t ms);
void bearer_adv_tx_timer_create(char *name, mesh_adv_tx_t *ptr,bool auto_reload,void (*cb)(mesh_timer_t));
bool bearer_adv_tx_timer_cancel(mesh_adv_tx_t *adv);
void bearer_on_adv_start_callback(mesh_adv_tx_t *adv);


#endif

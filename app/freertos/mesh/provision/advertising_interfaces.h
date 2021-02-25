
#ifndef ADVERTISING_INTERFACES_H_
#define ADVERTISING_INTERFACES_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "log.h"
#include "adv_bearer_tx.h"
#include "beacon.h"
#include "gap.h"
#include "co_endian.h"


typedef void (*advertising_done_callback_t)(uint8_t status, uint8_t , uint8_t);
typedef void (*on_advertising_start_callback_t)(mesh_adv_tx_t *);

typedef struct {
    advertising_done_callback_t adv_done_cb;
    on_advertising_start_callback_t on_start_cb;
    mesh_adv_tx_t adv_pdu;
    uint16_t adv_interval;    // ms
    uint8_t adv_count;
    uint8_t current_count;
    uint8_t advertising_need_to_stop;
} mesh_ble_adv_t;

enum mesh_adv_cmp_status
{
    MESH_ADV_COMPLETE,
    MESH_ADV_CANCELED,
};


/*************************************************************************************
 * 
 * Interfaces provided to  customer 
 */
void advertising_set_param(mesh_ble_adv_t *mesh_ble_adv_inst , uint16_t advertise_interval, uint8_t advertise_count,uint8_t adv_pkt_type, void (* adv_done_cb)(uint8_t, uint8_t, uint8_t),void (*on_start_cb)(mesh_adv_tx_t *));
void advertising_stop(mesh_ble_adv_t *mesh_ble_adv_inst );
void advertising_start(mesh_ble_adv_t *mesh_ble_adv_inst, uint8_t *adv_data,  uint8_t adv_data_length,mesh_adv_scan_rsp_t *scan_rsp);


#endif /* ADVERTISING_INTERFACES_H_ */

/*
 * 
 *
 *  Created on: 2019/8/30
 *  Author: 
 */
#define LOG_TAG "mesh_ble_adv"
#define LOG_LVL LVL_DBG
#include "bx_log.h"
#include "adv_bearer_tx.h"
#include "beacon.h"
#include "gap.h"
#include "co_endian.h"
#include "advertising_interfaces.h"
#include "mesh_sched.h"
#include "mesh_queued_msg.h"
#include "unprov_adv_interfaces.h"

/*************************************************************************************
 * LOCAL FUNCTIONS declaration
 *
 */


static uint8_t unprov_beacon_state = 0;
static mesh_ble_adv_t unprov_beacon_adv_inst = {
    .adv_interval = 5,
    .adv_count = 0,
};

uint8_t unprov_beacon_start(void (* unprov_beacon_start_done_callback)(uint8_t, uint8_t, uint8_t))
{
    if(unprov_beacon_state == 1)
        return 1;
    unprov_beacon_state = 1;
    advertising_set_param(&unprov_beacon_adv_inst, unprov_beacon_adv_inst.adv_interval, unprov_beacon_adv_inst.adv_count, UNPROV_DEV_BEACON_ADV_PKT, unprov_beacon_start_done_callback, NULL);
    advertising_start(&unprov_beacon_adv_inst, &unprov_beacon_data[0], 22, NULL);
    return 0;
}

uint8_t unprov_beacon_stop(void)
{
    unprov_beacon_state = 0;
    advertising_stop(&unprov_beacon_adv_inst);
    return 0;
}




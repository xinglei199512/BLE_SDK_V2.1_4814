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
#include "provision_service_adv_interfaces.h"


/*************************************************************************************
 * LOCAL FUNCTIONS declaration
 *
 */

static uint8_t provisioning_service_advertising_state = 0;
static mesh_ble_adv_t provisioning_service_adv_inst = {
    .adv_interval = 5,
    .adv_count = 0,
};


uint8_t provisioning_service_advertising_start(void (* provisioning_service_advertising_start_done_callback)(uint8_t, uint8_t, uint8_t))
{
    if(provisioning_service_advertising_state == 1)
        return 1;
    provisioning_service_advertising_state = 1;
    advertising_set_param(&provisioning_service_adv_inst, provisioning_service_adv_inst.adv_interval, provisioning_service_adv_inst.adv_count, GATT_SERVICE_ADV_PKT, provisioning_service_advertising_start_done_callback, NULL );
    advertising_start(&provisioning_service_adv_inst, &provisioning_service_advertising_data[0], 29, NULL);
    return 0;
}

uint8_t provisioning_service_advertising_stop(void)
{
    provisioning_service_advertising_state = 0;
    advertising_stop(&provisioning_service_adv_inst);
    return 0;
}



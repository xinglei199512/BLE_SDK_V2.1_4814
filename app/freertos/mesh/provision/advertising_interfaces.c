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


/*************************************************************************************
 * LOCAL FUNCTIONS declaration
 *
 */
static void advertising_start_tx_timer_handler_bottom_half(mesh_ble_adv_t *mesh_ble_adv_inst);
static void advertising_start_tx_timer_handler_top_half(mesh_timer_t xTimer);

void advertising_set_param(mesh_ble_adv_t *mesh_ble_adv_inst , uint16_t advertise_interval, uint8_t advertise_count,uint8_t adv_pkt_type, void (* adv_done_cb)(uint8_t, uint8_t, uint8_t),void (*on_start_cb)(mesh_adv_tx_t *))
{
    mesh_ble_adv_inst->advertising_need_to_stop = 0;   
    mesh_ble_adv_inst->adv_count = advertise_count;
    mesh_ble_adv_inst->adv_interval = advertise_interval;
    mesh_ble_adv_inst->current_count = 0;
    mesh_ble_adv_inst->adv_pdu.pkt_type = adv_pkt_type;
    mesh_ble_adv_inst->adv_done_cb = adv_done_cb;
    mesh_ble_adv_inst->on_start_cb = on_start_cb;

}

void advertising_stop(mesh_ble_adv_t *mesh_ble_adv_inst)
{
    mesh_ble_adv_inst->advertising_need_to_stop = 1;
}

void advertising_start(mesh_ble_adv_t *mesh_ble_adv_inst, uint8_t *adv_data,  uint8_t adv_data_length,mesh_adv_scan_rsp_t *scan_rsp)
{
    mesh_ble_adv_inst->adv_pdu.scan_rsp = scan_rsp;
    mesh_ble_adv_inst->adv_pdu.data_length = adv_data_length ;
    memcpy((uint8_t *)mesh_ble_adv_inst->adv_pdu.adv_data, adv_data, adv_data_length);
    bearer_adv_tx_timer_create("advertising_start_tx", &(mesh_ble_adv_inst->adv_pdu), false, advertising_start_tx_timer_handler_top_half);
    mesh_ble_adv_inst->current_count++;
    mesh_sched_adv_tx(&(mesh_ble_adv_inst->adv_pdu), 0);
    bearer_adv_tx_timer_start(&(mesh_ble_adv_inst->adv_pdu), mesh_ble_adv_inst->adv_interval);

}

static void advertising_start_tx_timer_handler_bottom_half(mesh_ble_adv_t *mesh_ble_adv_inst)
{
    LOG_D("[advertisingInterface] advertising_start_tx_timer_handler_bottom_half [enter] \n");

    if(mesh_ble_adv_inst->advertising_need_to_stop == 1)
    {
        mesh_ble_adv_inst->adv_done_cb(MESH_ADV_CANCELED, mesh_ble_adv_inst->current_count, mesh_ble_adv_inst->adv_count);
        return;
    }

    if(mesh_ble_adv_inst->adv_count == 0)
    {
        LOG_D("[advertisingInterface] adv until advertising_need_to_stop is set \n");

        mesh_sched_adv_tx(&(mesh_ble_adv_inst->adv_pdu), 0);
        bearer_adv_tx_timer_start(&(mesh_ble_adv_inst->adv_pdu), mesh_ble_adv_inst->adv_interval);
        return;
    }

    if(mesh_ble_adv_inst->current_count < mesh_ble_adv_inst->adv_count  )
    {
        LOG_D("[advertisingInterface] current_count %d < adv_count %d \n", mesh_ble_adv_inst->current_count, mesh_ble_adv_inst->adv_count  );
        mesh_ble_adv_inst->current_count++;

        mesh_sched_adv_tx(&(mesh_ble_adv_inst->adv_pdu), 0);
        bearer_adv_tx_timer_start(&(mesh_ble_adv_inst->adv_pdu), mesh_ble_adv_inst->adv_interval);
    }else
    {
        LOG_D("[advertisingInterface] advertising_start_tx_timer_handler_bottom_half: notify user adv count reached\n");
        mesh_ble_adv_inst->adv_done_cb(MESH_ADV_COMPLETE, mesh_ble_adv_inst->current_count, mesh_ble_adv_inst->adv_count );
    }
}

static void advertising_start_tx_timer_handler_top_half(mesh_timer_t xTimer)
{
    mesh_adv_tx_t *adv_pdu = mesh_timer_get_associated_data(xTimer);
    LOG_D("[advertisingInterface] advertising_start_tx_timer_handler_top_half [enter] \n");
    mesh_ble_adv_t *ptr = CONTAINER_OF(adv_pdu, mesh_ble_adv_t, adv_pdu);
    mesh_queued_msg_send( (void (*)(void *)) advertising_start_tx_timer_handler_bottom_half, ptr);
    LOG_D("[advertisingInterface] advertising_start_tx_timer_handler_top_half [out] \n");
}





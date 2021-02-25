/**
 ****************************************************************************************
 *
 * @file   light_lightness_setup_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-21 17:17
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "light_lightness_setup_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
void light_lightness_setup_server_action(light_lightness_setup_server_t *server, uint16_t target_value, mesh_light_lightness_model_evt_type_t type)
{
    mesh_model_evt_t evt;

    memset(&evt, 0, sizeof(mesh_model_evt_t));

    evt.type.lightness_type = type;
    evt.params.model_value_set.target_value = target_value;

    if(server->cb)
        server->cb(&evt);
}
static void light_lightness_default_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_lightness_setup_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_setup_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);


    server->tid_queue.inst_param.inst = (void *)server;

    server->msg_format->lightness_default = *(uint16_t *)(access + 2);
    if(server->msg_format->lightness_default < server->msg_format->lightness_range_min)
        server->msg_format->lightness_default = server->msg_format->lightness_range_min;
    else if(server->msg_format->lightness_default > server->msg_format->lightness_range_max)
        server->msg_format->lightness_default = server->msg_format->lightness_range_max;

    light_lightness_setup_server_action(server, server->msg_format->lightness_default, LIGHTNESS_DEFAULT_EVT_SET);
}

static void light_ligntness_default_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "light_ligntness_default_tx_done\n");
}

void send_light_lightness_default(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint32_t lightness_actual, uint16_t opcode_offset)
{
    light_lightness_actual_default_status_t msg;
    light_lightness_setup_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_setup_server_t, model);
    msg.lightness_actual = lightness_actual;
    if(msg.lightness_actual == 0)
        LOG(3, "send_light_lightness_default ERR\n");

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET, opcode_offset);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_lightness_actual_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ligntness_default_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

void light_lightness_Default_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "light_lightness_Default_get_rx\n");
    light_lightness_setup_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_setup_server_t, model);
    send_light_lightness_default(elmt, model, pdu, server->msg_format->lightness_default, Light_Lightness_Default_Status);
}

void light_lightness_Default_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_lightness_setup_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_setup_server_t, model);

    light_lightness_default_set_handle(elmt, model, pdu);

    send_light_lightness_default(elmt, model, pdu, server->msg_format->lightness_default, Light_Lightness_Default_Status);
}

void light_lightness_Default_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_lightness_default_set_handle(elmt, model, pdu);
}

static int light_lightness_range_set_handle(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_lightness_setup_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_setup_server_t, model);
    uint8_t * access = access_get_pdu_payload(pdu);
    int ret = 0;
    uint16_t range_min, range_max;

    server->tid_queue.inst_param.inst = (void *)server;

    range_min = *(uint16_t *)(access + 2);
    range_max = *(uint16_t *)(access + 4);
    server->msg_format->lightness_range_min = *(uint16_t *)(access + 2);
    server->msg_format->lightness_range_max = *(uint16_t *)(access + 4);

    if(range_min == 0) {
    //    server->msg_format->status_code = 1;
        ret = -1;
    }
    else if(range_min > range_max) {
    //    server->msg_format->status_code = 2;
        ret = -2;
    }
    else {
        server->msg_format->status_code = 0;
        server->msg_format->lightness_range_min = range_min;
        server->msg_format->lightness_range_max = range_max;
        ret = 0;
    }
    return ret;
}

static void light_ligntness_range_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "light_ligntness_last_tx_done\n");
}
void send_light_lightness_range(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_lightness_range_status_t msg;
    light_lightness_setup_server_t *server = GET_SERVER_MODEL_PTR(light_lightness_setup_server_t, model);
    msg.status_code = server->msg_format->status_code;
    msg.lightness_range_min = server->msg_format->lightness_range_min;
    msg.lightness_range_max = server->msg_format->lightness_range_max;
    if(msg.lightness_range_min == 0 || msg.lightness_range_min > msg.lightness_range_max) {
        LOG(3, "send_light_lightness_range ERR\n");
        msg.lightness_range_min = 0x1;
        msg.lightness_range_max = 0xffff;
    }
    
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(LIGHT_LIGHTNESS_TWO_OCTETS_OPCODE_OFFSET, Light_Lightness_Range_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(light_lightness_range_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ligntness_range_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
    
}

void light_lightness_Range_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "light_lightness_Range_get_rx\n");
    send_light_lightness_range(elmt, model, pdu);
}

void light_lightness_Range_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    int ret = light_lightness_range_set_handle(elmt, model, pdu);
    if(ret == 0)
        send_light_lightness_range(elmt, model, pdu);
}

void light_lightness_Range_set_unacknowledged_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_lightness_range_set_handle(elmt, model, pdu);
}


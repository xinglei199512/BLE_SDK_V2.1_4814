/**
 ****************************************************************************************
 *
 * @file   time_client.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-12-17 11:32
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

#include "time_client.h"
#include "time_server.h"
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

void time_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    time_server_t *server = GET_SERVER_MODEL_PTR(time_server_t, model);
    uint8_t time_role = server->msg_format->role_set.time_role;
    uint8_t * access = access_get_pdu_payload(pdu);

    LOG(3, "%s time_role:%d\n", __func__, time_role);

    if(time_role == MESH_TIME_RELAY || time_role == MESH_TIME_CLIENT) {
        time_set_t *p_pdu = (time_set_t *)(access + 1);
        server->msg_format->set.tai_seconds = p_pdu->tai_seconds;
        server->msg_format->set.subsecond = p_pdu->subsecond;
        server->msg_format->set.uncertainty = p_pdu->uncertainty;
        server->msg_format->set.time_authority = p_pdu->time_authority;
        server->msg_format->set.tai_utc_delta = p_pdu->tai_utc_delta;
        server->msg_format->set.time_zone_offset = p_pdu->time_zone_offset;
    }
}

void time_role_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}
void time_zone_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}
void tai_utc_delta_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}

static void time_msg_publish_done(void *pdu, uint8_t status)
{
    LOG(3, "time_msg_publish_done\n");
}
void time_msg_publish(time_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode)
{
    uint16_t appkey_global_idx = client->model.base.publish->appkey_idx;
    LOG(3,"time_msg_publish\n");
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msgLen;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,time_msg_publish_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

/**
 ****************************************************************************************
 *
 * @file   light_ctl_client.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 19:03
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

#include "light_ctl_client.h"
#include "access_rx_process.h"
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

void light_CTL_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_ctl_client_t *client = GET_SERVER_MODEL_PTR(light_ctl_client_t, model);
    LOG(3, "light_CTL_status_rx\n");

    if(client->get_current_state_cb) {
        uint8_t status = client->get_current_state_cb();

        if(status == WAIT_FOR_LIGHT_CTL_STATUS) {
            uint16_t present_value = 0;
            uint8_t payload_size = get_access_pdu_rx_payload_size(pdu); // opcode=2byte , transmic=4byte
            uint8_t * access = access_get_pdu_payload(pdu);

            LOG(3, "%s payload_size:%d\n", __func__, payload_size);
            if(payload_size == sizeof(light_ctl_default_status_t))
                present_value = *(uint16_t *)(access + 4);
            else if(payload_size == sizeof(light_ctl_status_t))
                present_value = *(uint16_t *)(access + 8);

            if(client->set_current_value_cb)
                client->set_current_value_cb(status, (uint16_t)present_value);
        }
    }
}
void light_CTL_temperature_range_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "light_CTL_temperature_range_status_rx\n");
}
void light_CTL_temperature_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "light_CTL_temperature_status_rx\n");
}
void light_CTL_Default_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "light_CTL_Default_status_rx\n");
}

static void light_ctl_msg_publish_done(void *pdu, uint8_t status)
{
    LOG(3, "light_ctl_msg_publish_done\n");
}
void light_ctl_msg_publish(light_ctl_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode)
{
    uint16_t appkey_global_idx = client->model.base.publish->appkey_idx;
    LOG(3,"light_ctl_msg_publish\n");

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msgLen;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 0;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_ctl_msg_publish_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}


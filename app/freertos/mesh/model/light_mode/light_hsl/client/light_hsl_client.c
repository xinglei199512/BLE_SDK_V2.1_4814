/**
 ****************************************************************************************
 *
 * @file   light_hsl_client.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2018-11-27 19:04
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

#include "light_hsl_client.h"
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

void light_HSL_hue_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}
void light_HSL_saturation_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}
void light_HSL_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    light_hsl_client_t *client = GET_SERVER_MODEL_PTR(light_hsl_client_t, model);
    LOG(3, "light_HSL_status_rx\n");

    if(client->get_current_state_cb) {
        uint8_t status = client->get_current_state_cb();

        if(status == WAIT_FOR_LIGHT_HSL_STATUS) {
            uint16_t present_value = 0;
            uint8_t payload_size = get_access_pdu_rx_payload_size(pdu); // opcode=2byte , transmic=4byte
            /* TODO */
            uint8_t * access = access_get_pdu_payload(pdu);
            present_value = *(uint16_t *)(access + 2);
            if(client->set_current_value_cb)
                client->set_current_value_cb(status, (uint16_t)present_value);
        }
    }
}
void light_HSL_target_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}
void light_HSL_default_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}
void light_HSL_Range_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "%s\n", __func__);
}

static void light_hsl_msg_publish_done(void *pdu, uint8_t status)
{
    LOG(3, "light_hsl_msg_publish_done\n");
}
void light_hsl_msg_publish(light_hsl_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode)
{
    uint16_t appkey_global_idx = client->model.base.publish->appkey_idx;
    LOG(3,"light_hsl_msg_publish\n");
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msgLen;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,light_hsl_msg_publish_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

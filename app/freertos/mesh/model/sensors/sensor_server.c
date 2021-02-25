/**
 ****************************************************************************************
 *
 * @file   sensor_server.c
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2019-02-25 11:07
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "sensor_server.h"
#include "app_keys_dm.h"
#include "access_tx_process.h"

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
static void sensor_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3, "sensor_status_tx_done\n");
}

static access_pdu_tx_t* sensor_get_access_pdu_tx_t(uint16_t src_addr, uint16_t dst_addr, void **msg, uint16_t msg_len, uint16_t global_idx, uint16_t opcode, void(*cb)(void *, uint8_t))
{
    model_tx_msg_param_t model_tx_msg_param;
    access_pdu_tx_t *access_pdu_tx_var;

    memset(&model_tx_msg_param,0,sizeof(model_tx_msg_param_t));
    model_tx_msg_param.dst_addr.addr = dst_addr;
    model_tx_msg_param.src_addr = src_addr;
    model_tx_msg_param.opcode = opcode;
    model_tx_msg_param.pdu_length = msg_len;
    model_tx_msg_param.seg = true;
    model_tx_msg_param.akf = true;
    dm_appkey_index_to_appkey_handle(global_idx ,&model_tx_msg_param.key.app_key);

    access_pdu_tx_var = access_model_msg_build(&model_tx_msg_param, cb);
    if(opcode > 0xc00000) {
        *msg = (void *)(access_pdu_tx_var->src + 3);
    }
    else if(opcode > 0xFF)
    {
        *msg = (void *)(access_pdu_tx_var->src + 2);
    }
    else
    {
        *msg = (void *)(access_pdu_tx_var->src + 1);
    }

    return access_pdu_tx_var;
#if 0
    access_model_pkt_fill_payload(access_pdu_tx_var, model_tx_msg_param.opcode, msg, model_tx_msg_param.pdu_length);

    if(NULL == access_pdu_tx_var)
    {
        return ;
    }
    access_send(access_pdu_tx_var);
#endif
}

static int get_property_id_index(sensor_server_t *server, uint16_t property_id)
{
    int index = -1;
    for(int i = 0; i < SENSOR_NUM; i++) {
        if(server->msg_format[i].property_id == property_id)
            index = i;
    }
    return index;
}
static void sensor_send_descriptor_status(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu) 
{
    sensor_server_t *server = GET_SERVER_MODEL_PTR(sensor_server_t, model);
    uint16_t payload_size = get_access_pdu_rx_payload_size(pdu);
    uint8_t *access = access_get_pdu_payload(pdu);
    uint16_t property_id = *(uint16_t *)(access + 2);
    sensor_descriptor_status_t *msg;
    int index = get_property_id_index(server, property_id);
    uint32_t opcode = TWO_OCTETS_OPCODE_GEN(SENSOR_ONE_OCTET_OPCODE_OFFSET, Sensor_Descriptor_Status);
    access_pdu_tx_t *access_pdu_tx_var;
    uint16_t msg_len;

    if(payload_size  == 2 && index != -1) {
        msg_len = 2;
        access_pdu_tx_var = sensor_get_access_pdu_tx_t(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void **)&msg, msg_len, access_get_pdu_appkey_global_index(pdu), opcode, sensor_status_tx_done);
        msg->descriptor[0] = server->msg_format[0].descriptor;
    }else {
        int i = 0;
        msg_len = 2 * SENSOR_NUM;
        access_pdu_tx_var = sensor_get_access_pdu_tx_t(elmt->uni_addr, access_get_pdu_src_addr(pdu), (void **)&msg, msg_len, access_get_pdu_appkey_global_index(pdu), opcode, sensor_status_tx_done);
        while(i < SENSOR_NUM) {
            msg->descriptor[i] = server->msg_format[i].descriptor;
            i++;
        }
    }

    if(NULL == access_pdu_tx_var)
    {
        return ;
    }
    access_send(access_pdu_tx_var);
}

void sensor_descriptor_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    sensor_send_descriptor_status(elmt, model, pdu);
}

void sensor_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
}
void sensor_colunm_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
}
void sensor_series_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
}


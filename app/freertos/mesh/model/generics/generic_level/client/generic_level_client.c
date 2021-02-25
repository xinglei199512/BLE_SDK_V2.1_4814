#include "osapp_config.h"
#include "generic_level_client.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"

static void generic_level_get_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"generic_level_get_tx_done\n");
}

static void generic_level_status_rx_done(uint8_t status, uint8_t *value)
{
    LOG(3,"generic_level_status_rx\n");
    if(status == GENERIC_LEVEL_DEFAULT_STATUS)
    {
        LOG(3,"Present level = 0x%x\n", *(uint16_t *)(value+2));
    }
    if(status == GENERIC_LEVEL_STATUS)
    {
        LOG(3,"Present level = 0x%x\n", *(uint16_t *)(value+2));
        LOG(3,"Target level = 0x%x\n",  *(uint16_t *)(value+4));
        LOG(3,"Remaining Time = 0x%x\n", *(value+6));
    }
}

void generic_level_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    generic_level_client_t *client = GET_SERVER_MODEL_PTR(generic_level_client_t,model);
    LOG(3, "generic_level_status_rx\n");
    if(client->get_current_state_cb) {
        uint8_t status = client->get_current_state_cb();
        if(status == WAIT_FOR_LIGHT_HSL_STATUS) {
            uint16_t present_value = 0;
            uint8_t payload_size = get_access_pdu_rx_payload_size(pdu); // opcode=2byte , transmic=4byte
            uint8_t * access = access_get_pdu_payload(pdu);
            if(payload_size == sizeof(generic_level_msg_default_t))
            {
                present_value = *(uint16_t *)(access + 2);
                generic_level_status_rx_done(GENERIC_LEVEL_DEFAULT_STATUS, (void *)access);
            }
            else if(payload_size == sizeof(generic_level_msg_status_t))
            {
                present_value = *(uint16_t *)(access + 4);
                generic_level_status_rx_done(GENERIC_LEVEL_STATUS, (void *)access);
            }

            if(client->set_current_value_cb)
                client->set_current_value_cb(status, (uint16_t)present_value);
        }
    }
}

void generic_level_get_tx(generic_level_client_t *client, void *msg , uint8_t msg_length, uint16_t dst_addr, uint16_t appkey_global_idx)
{
    LOG(3,"generic_level_get_tx dst_addr:%x msg_length:%d idx:%d\n", dst_addr, msg_length, appkey_global_idx);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_LEVEL_OPCODE_OFFSET , Generic_Level_Get);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msg_length;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_level_get_tx_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static void generic_level_msg_publish_done(void *pdu, uint8_t status)
{
    LOG(3,"generic_level_msg_publish_done\n");
}

void generic_level_msg_publish(generic_level_client_t *client, void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode)
{
    uint16_t appkey_global_idx = client->model.base.publish->appkey_idx;
    LOG(3,"generic_level_msg_publish\n");
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msgLen;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param, generic_level_msg_publish_done, (uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}


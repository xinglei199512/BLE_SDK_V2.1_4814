#include "osapp_config.h"
#include "generic_onoff_client.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include "generic_onoff_common.h"
#include "generic_onoff_msg_handler.h"

static void generic_onoff_get_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"generic_onoff_get_tx_done\n");
}

static void generic_onoff_status_rx_done(uint8_t status , uint8_t * value)
{
    LOG(3,"generic_onoff_status_rx\n");
    if(status == GENERIC_ONOFF_DEFAULT_STATUS)
    {
        LOG(3,"Present Onoff = 0x%x\n",*(uint8_t *)(value+2));//82 04 00
    }
    if(status == GENERIC_ONOFF_STATUS)
    {
        LOG(3,"Present Onoff = 0x%x\n", *(uint8_t *)(value+2));
        LOG(3,"Target Onoff = 0x%x\n",  *(uint8_t *)(value+3));
        LOG(3,"Remaining Time = 0x%x\n",*(uint8_t *)(value+4));
    }
}
void generic_onoff_status_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    generic_onoff_client_t *client = GET_SERVER_MODEL_PTR(generic_onoff_client_t,model);
    LOG(3, "generic_onoff_status_rx\n");

    if(client->get_current_state_cb) {
        uint8_t status = client->get_current_state_cb();

        if(status == WAIT_FOR_GENERIC_ONOFF_STATUS) {
            uint16_t present_value = 0;
            uint8_t payload_size = get_access_pdu_rx_payload_size(pdu); // opcode=2byte , transmic=4byte
            uint8_t * access = access_get_pdu_payload(pdu);
            if(payload_size == sizeof(generic_onoff_msg_default_state_t))
            {
                present_value = *(uint8_t *)(access + 2);
                generic_onoff_status_rx_done(GENERIC_ONOFF_DEFAULT_STATUS , (void *)access);
            }
            else if(payload_size == sizeof(generic_onoff_msg_state_t))
            {
                present_value = *(uint8_t *)(access + 3);
                generic_onoff_status_rx_done(GENERIC_ONOFF_STATUS , (void *)access);
            }
            if(client->set_current_value_cb)
                client->set_current_value_cb(status, (uint16_t)present_value);
        }
    }
    
}
void generic_onoff_get_tx(generic_onoff_client_t *client,void *msg , uint8_t msg_length,uint16_t dst_addr,uint16_t appkey_global_idx)
{
    LOG(3,"generic_onoff_get_tx\n");
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_ONOFF_OPCODE_OFFSET , Generic_OnOff_Get);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(generic_onoff_msg_default_state_t);
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onoff_get_tx_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

static void generic_onoff_msg_publish_done(void *pdu,uint8_t status)
{
    LOG(3,"generic_onoff_msg_publish_done\n");
}

void generic_onoff_msg_publish(generic_onoff_client_t *client,void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode)
{
    uint16_t appkey_global_idx = client->model.base.publish->appkey_idx;
    LOG(3,"generic_onoff_msg_publish:dst_addr:%x\n", dst_addr);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msgLen;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onoff_msg_publish_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}





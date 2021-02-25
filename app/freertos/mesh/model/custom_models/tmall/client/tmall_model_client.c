#include "osapp_config.h"
//#include "access.h"
#include "mesh_model.h"
#include "mesh_env.h"
#include "generic_common.h"
#include "tmall_model_client.h"
#include "tmall_model_common.h"
#include "model_servers_events_api.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"

void tmall_message_attr_status_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "tmall_message_attr_status_rx\n");
}

static void custom_tmall_msg_publish_done(void *pdu,uint8_t status)
{
    LOG(3,"custom_tmall_msg_publish_done\n");
}


void custom_tmall_msg_publish(custom_tmall_client_t *client, void* msg, uint32_t msgLen, uint16_t dst_addr, uint32_t opcode)
{
    uint16_t appkey_global_idx = client->model.base.publish->appkey_idx;
    LOG(3,"custom_tmall_msg_publish\n");

    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = opcode;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = msgLen;
    tx_param.src_addr = client->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,custom_tmall_msg_publish_done,(uint8_t *)msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}


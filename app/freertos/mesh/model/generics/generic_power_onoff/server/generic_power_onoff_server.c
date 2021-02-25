#include "osapp_config.h"
#include "generic_power_onoff_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"

static void generic_onpowerup_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"generic_onpowerup_status_tx_done\n");
}

void generic_onpowerup_status_publish(generic_power_onoff_server_t *server)
{
    uint16_t dst_addr = 0;
    uint16_t appkey_global_idx = server->model.base.publish->appkey_idx;
    LOG(3,"generic_onpowerup_status_publish target_onpowerup:%d\n", server->msg_format->onpowerup);

    if(server->model.base.publish == NULL)
    {
        return;
    }

    if(server->model.base.publish->addr.is_virt)
    {
        dst_addr = server->model.base.publish->addr.addr.virt->virt_addr;
    }
    else
    {
	    dst_addr = server->model.base.publish->addr.addr.addr;
    }
    if(IS_UNASSIGNED_ADDR(dst_addr))
    {
        return;
    }
    generic_onpowerup_status_t msg;
    msg.onpowerup = server->msg_format->onpowerup;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_POWER_ONOFF_OPCODE_OFFSET , Generic_OnPowerUp_Status);;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(generic_onpowerup_status_t);;
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onpowerup_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void generic_onpowerup_status_tx(generic_power_onoff_server_t *server,uint16_t dst_addr,access_pdu_rx_t *pdu)
{
    generic_onpowerup_status_t msg;
    LOG(3, "generic_onpowerup_status_tx onpowerup:%d\n", server->msg_format->onpowerup);
    msg.onpowerup = server->msg_format->onpowerup;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_POWER_ONOFF_OPCODE_OFFSET, Generic_OnPowerUp_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.pdu_length = sizeof(generic_onpowerup_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onpowerup_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);
}

void onpowerup_server_onpowerup_action(generic_power_onoff_server_t *server)
{
    //user callback
    mesh_model_evt_t evt;
    evt.type.onpowerup_type = POWER_ONOFF_MODEL_EVT_SET;
    evt.params.model_value_set.target_value = server->msg_format->onpowerup;
    LOG(3, "%s: onpowerup:%d\n", __func__, server->msg_format->onpowerup);

    if(server->cb)
        server->cb(&evt);

}

void generic_onpowerup_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    generic_power_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_power_onoff_server_t,model);
    uint16_t apkey_global_idx = access_get_pdu_appkey_global_index(pdu);
    //user callback
    mesh_model_evt_t evt;
    evt.type.onpowerup_type = POWER_ONOFF_MODEL_EVT_GET;
    server->cb(&evt);
    //send status
    generic_onpowerup_status_tx(server,access_get_pdu_src_addr(pdu),pdu);
}

static int generic_onpowerup_set_handler(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    //uint16_t payload_size;

    LOG(3,"generic_onpowerup_set_handler mic_length:%d total_length:%d\n", access_get_pdu_mic_length(pdu), access_get_pdu_payload_length(pdu));
    uint8_t * access = access_get_pdu_payload(pdu);
    //payload_size = get_access_pdu_rx_payload_size(pdu); // opcode=2byte , transmic=4byte
    
    generic_power_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_power_onoff_server_t,model);

    generic_onpowerup_set_t *p_pdu = (generic_onpowerup_set_t *)(access + 2);

    if(p_pdu->onpowerup > 2)
    {
        LOG(3,"ERROR ONOFF VALUE\n");
        return -1;//discard this message,prohibit
    }

    //*publish_status = (server->msg_format->onpowerup != p_pdu->onpowerup);
    *publish_status = 1;
    server->msg_format->onpowerup = p_pdu->onpowerup;
    onpowerup_server_onpowerup_action(server);

    return 0;
}
void generic_onpowerup_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t apkey_global_idx = access_get_pdu_appkey_global_index(pdu);
    generic_power_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_power_onoff_server_t, model);
    uint8_t publish_status = 0;
    int ret;

    ret = generic_onpowerup_set_handler(elmt, model, pdu, &publish_status);
    //send ack
    LOG(3, "generic_onpowerup_set_rx publish_status:%d ret:%d\n", publish_status, ret);

    if(ret == 0)
        generic_onpowerup_status_tx(server,access_get_pdu_src_addr(pdu),pdu);

    if(publish_status) {
       // model_status_publish();
        generic_onpowerup_status_publish(server);
    }
}

void generic_onpowerup_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    generic_onpowerup_set_handler(elmt, model, pdu, &publish_status);
    generic_power_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_power_onoff_server_t,model);
    LOG(3, "generic_onpowerup_set_unacknowledged_rx publish_status:%d\n", publish_status);

    if(publish_status) {
        //model_status_publish();
        generic_onpowerup_status_publish(server);
    }
}


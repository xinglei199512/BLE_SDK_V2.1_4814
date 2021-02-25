#include "osapp_config.h"
#include "generic_onoff_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include "generic_onoff_common.h"
uint8_t tid2=0;
static void generic_onoff_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"generic_onoff_status_tx_done\n");
}

void generic_onoff_status_publish(generic_onoff_server_t *server)
{
    uint16_t dst_addr = 0;
    uint16_t appkey_global_idx = server->model.base.publish->appkey_idx;
    LOG(3,"generic_onoff_status_publish target_onoff:%d\n", server->msg_format.present_onoff);
	
    tid2++;
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
    LOG(3,"dst_addr :%x  \n", dst_addr);
    if(IS_UNASSIGNED_ADDR(dst_addr))
    {
        return;
    }
  //  generic_onoff_msg_default_state_t msg;
		generic_onoff_msg_state_t msg;
    msg.present_onoff = server->msg_format.present_onoff; 
		msg.target_onoff =server->msg_format.target_onoff;
		msg.remaining_time=model_transition_time_encode(0);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_ONOFF_OPCODE_OFFSET , Generic_OnOff_Status);;
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(generic_onoff_msg_default_state_t);;
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;	
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onoff_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
  	access_send(ptr);	
	
}
void generic_onoff_status_tx(generic_onoff_server_t *server,uint16_t dst_addr,access_pdu_rx_t *pdu)
{
    generic_onoff_msg_state_t msg1;
    generic_onoff_msg_default_state_t msg2;
    LOG(3, "generic_onoff_status_tx server_state:%d onoff:%d %d\n", server->server_state, server->msg_format.present_onoff, server->msg_format.target_onoff);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    tx_param.key.app_key = access_get_pdu_app_key(pdu);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_ONOFF_OPCODE_OFFSET,Generic_OnOff_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;

    if(server->server_state == GENERIC_TRANS_PROCESS)
    {
        uint32_t delay_trans_expiry = 0;
        tx_param.pdu_length = sizeof(generic_onoff_msg_state_t);
        msg1.target_onoff = server->msg_format.target_onoff;
        msg1.present_onoff = server->msg_format.present_onoff;

        delay_trans_expiry = generic_get_delay_trans_expiry(server->delay_trans_timer, server->delay_trans_timer->trans_time, server->delay_trans_timer->trans_timer_step);
        
        msg1.remaining_time =  model_transition_time_encode(delay_trans_expiry);
        LOG(3, "generic_onoff_status_tx remain_time:%x expiry:%x\n", msg1.remaining_time, delay_trans_expiry);
        access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onoff_status_tx_done,(uint8_t *)&msg1);
        BX_ASSERT(ptr);
        access_send(ptr);
    }
    else if(server->server_state == GENERIC_TRANS_IDALE)
    {
        tx_param.pdu_length = sizeof(generic_onoff_msg_default_state_t);
        msg2.present_onoff = server->msg_format.present_onoff;
        access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_onoff_status_tx_done,(uint8_t *)&msg2);
        BX_ASSERT(ptr);
        access_send(ptr);
    }
    
}

void onoff_server_onoff_action(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    generic_onoff_server_t *server = (generic_onoff_server_t *)timer_param->inst;

    LOG(3, "onoff_server_onoff_action remain_tick_count:%d server_state:%d\n", 
            server->delay_trans_timer->remain_tick_count, server->server_state);

    if(server->delay_trans_timer->remain_tick_count) {
    }else {
        //user callback
        mesh_model_evt_t evt;
        evt.type.onoff_type = ONOFF_MODEL_EVT_SET;
        evt.params.model_value_set.target_value = server->msg_format.target_onoff;

        if(server->cb)
            server->cb(&evt);

        if(server->delay_trans_timer->dt_timer_flag)
            server->server_state = GENERIC_TRANS_IDALE;
    }
}

void onoff_server_onoff_action_then_publish(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    generic_onoff_server_t *server = (generic_onoff_server_t *)timer_param->inst;

    onoff_server_onoff_action(param);

    if((server->delay_trans_timer->remain_tick_count == 0)) {
        model_status_publish();
    }
}

void generic_onoff_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;

    generic_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_onoff_server_t,model);
    uint16_t apkey_global_idx = access_get_pdu_appkey_global_index(pdu);
    //user callback
    mesh_model_evt_t evt;
    evt.type.onoff_type = ONOFF_MODEL_EVT_GET;
    server->cb(&evt);
    //send status
    generic_onoff_status_tx(server,access_get_pdu_src_addr(pdu),pdu);
}

static int generic_onoff_set_handler(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;

    LOG(3,"generic_onoff_set_handler mic_length:%d total_length:%d\n", access_get_pdu_mic_length(pdu), access_get_pdu_payload_length(pdu));
    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    uint8_t * access = access_get_pdu_payload(pdu);
    payload_size = get_access_pdu_rx_payload_size(pdu); // opcode=2byte , transmic=4byte
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);
    
    generic_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_onoff_server_t,model);

    server->tid_queue.inst_param.inst = (void *)server;
    if(payload_size == sizeof(generic_onoff_msg_set_t))
    {
        LOG(3,"generic_onoff_msg_set_t\n");
        generic_onoff_msg_set_t *p_pdu = (generic_onoff_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;
        if(p_pdu->onoff > 1) 
        {
            LOG(3,"ERROR ONOFF VALUE\n");
            return -1;//discard this message,prohibit
        }

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -2;
        }

        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format.target_onoff = p_pdu->onoff;
        server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;
        *publish_status = (server->msg_format.target_onoff != server->msg_format.present_onoff);
            
        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0) {
            server->delay_trans_timer->dt_timer_flag = USE_FOR_TRANS_TIME;
            onoff_server_onoff_action(server->delay_trans_timer);
        }else {
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->target_value = p_pdu->onoff;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_ONOFF;
            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, onoff_server_onoff_action_then_publish);
        }
    }
    else if(payload_size == sizeof(generic_onoff_msg_default_set_t))
    {
        LOG(3,"generic_onoff_msg_default_set_t\n");
        generic_onoff_msg_default_set_t *p_pdu = (generic_onoff_msg_default_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;
        if(p_pdu->onoff > 1)
        {
            LOG(3,"ERROR ONOFF VALUE\n");
            return -3;//discard this message,prohibit
        }

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -4;
        }

        server->server_state = GENERIC_TRANS_PROCESS;

        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
        server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_ONOFF;
        server->delay_trans_timer->inst = (void *)server;

        server->msg_format.target_onoff = p_pdu->onoff;
        *publish_status = (server->msg_format.target_onoff != server->msg_format.present_onoff);

        if(server->delay_trans_timer->trans_time == 0) {
            server->delay_trans_timer->dt_timer_flag = USE_FOR_TRANS_TIME;
            onoff_server_onoff_action(server->delay_trans_timer);
        }else {
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, onoff_server_onoff_action_then_publish);
        }
    }

    return 0;
}
void generic_onoff_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t apkey_global_idx = access_get_pdu_appkey_global_index(pdu);
    generic_onoff_server_t *server = GET_SERVER_MODEL_PTR(generic_onoff_server_t, model);
    uint8_t publish_status = 0;
    int ret;

    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;

    ret = generic_onoff_set_handler(elmt, model, pdu, &publish_status);
    //send ack
    LOG(3, "generic_onoff_set_rx publish_status:%d ret:%d\n", publish_status, ret);

    if(ret == 0)
        generic_onoff_status_tx(server,access_get_pdu_src_addr(pdu),pdu);

    if(publish_status) {
        model_status_publish();
    }
}
void generic_onoff_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;

    generic_onoff_set_handler(elmt, model, pdu, &publish_status);
    LOG(3, "generic_onoff_set_unacknowledged_rx publish_status:%d\n", publish_status);

    if(publish_status) {
        model_status_publish();
    }
}


#include "osapp_config.h"
#include "generic_level_server.h"
#include "access_tx_process.h"
#include "app_keys_dm.h"
#include "node_setup.h"

static void generic_level_status_tx_done(void *pdu,uint8_t status)
{
    LOG(3,"generic_level_status_tx_done\n");
}

void generic_level_status_publish(generic_level_server_t *server)
{
    uint16_t dst_addr = 0;
    uint16_t appkey_global_idx = server->model.base.publish->appkey_idx;

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
    LOG(3,"generic_level_status_publish present_level:%x dst_addr:%x\n", server->msg_format.present_level, dst_addr);
    generic_level_msg_default_status_t msg;
    msg.present_level = server->msg_format.present_level;
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle(appkey_global_idx ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_LEVEL_OPCODE_OFFSET , Generic_Level_Status);
    tx_param.dst_addr.addr = dst_addr;
    tx_param.pdu_length = sizeof(generic_level_msg_default_status_t);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_level_status_tx_done,(uint8_t *)&msg);
    BX_ASSERT(ptr);
    access_send(ptr);

}

static void level_server_level_action(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    generic_level_server_t *server = (generic_level_server_t *)timer_param->inst;
    uint16_t target_value;
    uint16_t present_lightness, target_lightness;
    int level_change_count = 0;
    int level_count = 0;
    uint8_t first_enter_flag = 0;
    uint16_t trans_timer_tmp;
    mesh_model_evt_t evt;

    if(server->delay_trans_timer->remain_tick_count) {
        first_enter_flag = (pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp))
                                    == (server->delay_trans_timer->remain_tick_count + pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step)));

        level_count = pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);

        level_change_count = level_count - server->delay_trans_timer->remain_tick_count / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);
        if(first_enter_flag == 1) {
            server->msg_format.origin_present_level = server->msg_format.present_level;
        }
    }else {
        level_change_count = level_count = 1;
    }
    present_lightness = server->msg_format.origin_present_level + 32768;
    target_lightness = server->msg_format.target_level + 32768;

    if(present_lightness > target_lightness) {
        target_value = (uint16_t)((present_lightness - target_lightness) * ((level_change_count * 1.0) / level_count));
        evt.params.model_value_set.target_value = (uint16_t)(present_lightness - 32768 - target_value);
    }else {
        target_value = (uint16_t)((target_lightness - present_lightness) * ((level_change_count * 1.0) / level_count));
        evt.params.model_value_set.target_value = (uint16_t)(present_lightness - 32768 + target_value);
    }

    evt.type.level_type = LEVEL_MODEL_EVT_SET;

    if(server->cb)
        server->cb(&evt);

    LOG(3,"level_server_action target_state:%x %x target_value:%x %x count:%d %d\n", 
            server->msg_format.target_level, server->msg_format.present_level, evt.params.model_value_set.target_value, target_value, level_change_count, level_count);
    if(server->msg_format.present_level == server->msg_format.target_level)
        server->server_state = GENERIC_TRANS_IDALE;
}

static void level_server_level_action_then_publish(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    generic_level_server_t *server = (generic_level_server_t *)timer_param->inst;

    level_server_level_action(param);
    
    if((server->delay_trans_timer->remain_tick_count == 0 && timer_param->dt_timer_flag == USE_FOR_TRANS_TIME)) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

static void level_server_move_action(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    generic_level_server_t *server = (generic_level_server_t *)timer_param->inst;
    int level_change_count = 0;
    int level_count = 0;
    uint8_t first_enter_flag = 0;
    uint16_t trans_timer_tmp;
    mesh_model_evt_t evt;
    uint16_t target_value = 0;

    if(server->delay_trans_timer->remain_tick_count) {
        first_enter_flag = (pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp))
                                    == (server->delay_trans_timer->remain_tick_count + pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step)));

        level_count = pdMS_TO_TICKS(model_transition_time_decode(server->delay_trans_timer->trans_time, &trans_timer_tmp)) / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);

        level_change_count = level_count - server->delay_trans_timer->remain_tick_count / pdMS_TO_TICKS(server->delay_trans_timer->trans_timer_step);
        if(first_enter_flag == 1) {
            server->msg_format.origin_present_level = server->msg_format.present_level;
        }
    }else {
        level_change_count = level_count = 1;
    }
    target_value = server->msg_format.present_level;
    //target_value += server->msg_format.params.move.required_move;

    evt.type.level_type = LEVEL_MODEL_EVT_SET;
    evt.params.model_value_set.target_value = target_value;

    if(server->cb)
        server->cb(&evt);

    LOG(3,"level_server_move_action target_state:%x %x required_move:%x count:%d %d\n", 
            server->msg_format.target_level, server->msg_format.present_level, server->msg_format.params.move.required_move, level_change_count, level_count);
    if(level_count == level_change_count)
        server->server_state = GENERIC_TRANS_IDALE;
}
static void level_server_move_action_then_publish(void *param)
{
    generic_delay_trans_param_t *timer_param = (generic_delay_trans_param_t *)param;
    generic_level_server_t *server = (generic_level_server_t *)timer_param->inst;

    level_server_move_action(param);
    
    if((server->delay_trans_timer->remain_tick_count == 0)) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

static void send_generic_level_status(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    generic_level_msg_status_t msg1;
    generic_level_msg_default_status_t msg2;
    generic_level_server_t *server = GET_SERVER_MODEL_PTR(generic_level_server_t, model);
    LOG(3,"src_addr:%x state:%x present_level:%x target_level:%x\n", access_get_pdu_src_addr(pdu), server->server_state, server->msg_format.present_level, server->msg_format.target_level);
    model_tx_msg_param_t tx_param;
    memset(&tx_param,0,sizeof(model_tx_msg_param_t));
    dm_appkey_index_to_appkey_handle( access_get_pdu_appkey_global_index(pdu) ,&tx_param.key.app_key);
    tx_param.opcode = TWO_OCTETS_OPCODE_GEN(GENERIC_LEVEL_OPCODE_OFFSET, Generic_Level_Status);
    tx_param.dst_addr.addr = access_get_pdu_src_addr(pdu);
    tx_param.src_addr = server->model.base.elmt->uni_addr;
    tx_param.akf = 1;
    tx_param.seg = 1;
    ble_txrx_time_t rx_time  = access_rx_get_rx_time(pdu);
    tx_param.rx_time = &rx_time;

    if(server->server_state == GENERIC_TRANS_PROCESS) {
        uint32_t delay_trans_expiry = 0;
        msg1.present_level = server->msg_format.present_level;
        msg1.target_level = server->msg_format.target_level;

        delay_trans_expiry = generic_get_delay_trans_expiry(
                server->delay_trans_timer, server->delay_trans_timer->trans_time, server->delay_trans_timer->trans_timer_step);

        msg1.remaining_time = model_transition_time_encode(delay_trans_expiry);

        if(msg1.remaining_time & 0x80)
            msg1.remaining_time = 0x3f;

        LOG(3, "generic_level_status_tx remain_time:%x expiry:%x flag:%d\n", 
                msg1.remaining_time, delay_trans_expiry, server->delay_trans_timer->trans_timer_step);
        tx_param.pdu_length = sizeof(generic_level_msg_status_t);
        access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_level_status_tx_done,(uint8_t *)&msg1);
        BX_ASSERT(ptr);
        access_send(ptr);

    }else if(server->server_state == GENERIC_TRANS_IDALE) {
        tx_param.pdu_length = sizeof(generic_level_msg_default_status_t);

        msg2.present_level = server->msg_format.present_level;
        access_pdu_tx_t * ptr = access_model_pkt_build_fill(&tx_param,generic_level_status_tx_done,(uint8_t *)&msg2);
        BX_ASSERT(ptr);
        access_send(ptr);

    }
    
}

void generic_level_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    if(mesh_node_get_primary_element_addr() == access_get_pdu_src_addr(pdu))
        return;
    LOG(3,"generic_level_get_rx\n");
    mesh_model_evt_t evt;
    generic_level_server_t *server = GET_SERVER_MODEL_PTR(generic_level_server_t, model);
    evt.type.level_type = LEVEL_MODEL_EVT_GET;
    if(server->cb)
        server->cb(&evt);

    send_generic_level_status(elmt, model, pdu);
}

int generic_level_set_rx_handler(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;
    uint8_t *access = access_get_pdu_payload(pdu);

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    generic_level_server_t *server = GET_SERVER_MODEL_PTR(generic_level_server_t, model);

    server->tid_queue.inst_param.inst = (void *)server;
    server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;

    LOG(3,"generic_level_set_rx_handler payload_size:%d dst:%x src:%x\n", payload_size, msg_field.dst, msg_field.src);

    if(payload_size == sizeof(generic_level_msg_set_t))
    {
        generic_level_msg_set_t *p_pdu = (generic_level_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -1;
        }

        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format.target_level = p_pdu->level;
        *publish_status = (server->msg_format.target_level != server->msg_format.present_level);

        LOG(3,"generic_level_set_rx_handler present_level:%x target_level:%x ticks:%d level:%x\n", server->msg_format.present_level, server->msg_format.target_level, pdMS_TO_TICKS(p_pdu->delay * 5), p_pdu->level);

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        LOG(3,"level:%x tid:%x trans_time:%x delay:%x server->cb:%p\n", p_pdu->level, p_pdu->tid, p_pdu->trans_time, p_pdu->delay, server->cb);

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){//this block code is used to transition state
            level_server_level_action(server->delay_trans_timer);
        }else {
            server->delay_trans_timer->target_value = p_pdu->level;
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LEVEL;

            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, level_server_level_action_then_publish);
        }

    }
    else if(payload_size == sizeof(generic_level_msg_default_t))
    {
        generic_level_msg_set_t *p_pdu = (generic_level_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -2;
        }

        //this block code is used to  store valid message  within 6 seconds

        server->server_state = GENERIC_TRANS_PROCESS;
        server->msg_format.target_level = p_pdu->level;
        *publish_status = (server->msg_format.target_level != server->msg_format.present_level);

        LOG(3,"2generic_level_set_rx_handler present_level:%x target_level:%x ticks:%d level:%x\n", server->msg_format.present_level, server->msg_format.target_level, pdMS_TO_TICKS(p_pdu->delay * 5), p_pdu->level);

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        //LOG(3,"level:%x tid:%x trans_time:%x delay:%x\n", p_pdu->level, p_pdu->tid, p_pdu->trans_time, p_pdu->delay);

        server->delay_trans_timer->target_value = p_pdu->level;
        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
        server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_LEVEL;

        if(server->delay_trans_timer->trans_time == 0)
            level_server_level_action(server->delay_trans_timer);
        else
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, level_server_level_action_then_publish);
    }

    return 0;
}
void generic_level_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    int ret;

    ret = generic_level_set_rx_handler(elmt, model, pdu, &publish_status);

    LOG(3,"generic_level_set_rx ret:%d publish_status:%d\n", ret, publish_status);
    if(ret == 0)
        send_generic_level_status(elmt, model, pdu);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

void generic_level_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;

    generic_level_set_rx_handler(elmt, model, pdu, &publish_status);

    LOG(3,"generic_level_set_unacknowledged_rx publish_status:%d\n", publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}
void generic_delta_set_rx_handler(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;
    uint8_t *access = access_get_pdu_payload(pdu);

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    generic_level_server_t *server = GET_SERVER_MODEL_PTR(generic_level_server_t,model);
    server->tid_queue.inst_param.inst = (void *)server;

    if(payload_size == sizeof(generic_delta_msg_set_t))
    {
        generic_delta_msg_set_t *p_pdu = (generic_delta_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(server->msg_format.params.set.required_delta == p_pdu->delta_level) {
            server->msg_format.params.set.required_delta = 0;
            server->msg_format.delta_last_tid = -1;
        }

        LOG(3, "1level:%x %x %x %x tid:%x %x\n", server->msg_format.target_level, p_pdu->delta_level, server->msg_format.present_level, server->msg_format.params.set.required_delta, msg_field.tid, server->msg_format.delta_last_tid);

        if(server->msg_format.delta_last_tid == msg_field.tid) {
            server->msg_format.target_level = server->msg_format.present_level - server->msg_format.params.set.required_delta + p_pdu->delta_level;
            server->msg_format.params.set.required_delta = p_pdu->delta_level;
        }else {
            server->msg_format.target_level = server->msg_format.present_level + p_pdu->delta_level;
            server->msg_format.params.set.required_delta = p_pdu->delta_level;
        }

        server->msg_format.delta_last_tid = msg_field.tid;

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        *publish_status = (server->msg_format.target_level != server->msg_format.present_level);

        server->server_state = GENERIC_TRANS_PROCESS;

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){
            level_server_level_action(server->delay_trans_timer);
        }else {
            server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;
            server->delay_trans_timer->target_value = server->msg_format.target_level;
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_DELTA;

            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, level_server_level_action_then_publish);
        }

    }
    else if(payload_size == sizeof(generic_delta_default_msg_set_t))
    {
        generic_delta_default_msg_set_t *p_pdu = (generic_delta_default_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(server->msg_format.params.set.required_delta == p_pdu->delta_level) {
            server->msg_format.params.set.required_delta = 0;
            server->msg_format.delta_last_tid = -1;
        }

        LOG(3, "level:%x %x %x %x tid:%x %x\n", server->msg_format.target_level, p_pdu->delta_level, server->msg_format.present_level, server->msg_format.params.set.required_delta, msg_field.tid, server->msg_format.delta_last_tid);

        if(server->msg_format.delta_last_tid == msg_field.tid) {
            server->msg_format.target_level = server->msg_format.present_level - server->msg_format.params.set.required_delta + p_pdu->delta_level;
            server->msg_format.params.set.required_delta = p_pdu->delta_level;
        }else {
            server->msg_format.target_level = server->msg_format.present_level + p_pdu->delta_level;
            server->msg_format.params.set.required_delta = p_pdu->delta_level;
        }
        server->msg_format.delta_last_tid = msg_field.tid;

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        *publish_status = (server->msg_format.target_level != server->msg_format.present_level);

        server->server_state = GENERIC_TRANS_PROCESS;

        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);

        if(server->delay_trans_timer->trans_time == 0)
            level_server_level_action(server->delay_trans_timer);
        else
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, level_server_level_action_then_publish);
    }

}

void generic_delta_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    LOG(3,"generic_delta_set_rx\n");
    generic_delta_set_rx_handler(elmt, model, pdu, &publish_status);

    send_generic_level_status(elmt, model, pdu);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}

void generic_delta_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    LOG(3,"generic_delta_set_unacknowledged_rx\n");
    generic_delta_set_rx_handler(elmt, model, pdu, &publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}



static int  generic_move_set_rx_handler(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu, uint8_t *publish_status)
{
    uint16_t payload_size;
    uint8_t *access = access_get_pdu_payload(pdu);

    generic_valid_field_check_t msg_field;// store src ,dst ,tid
    payload_size = get_access_pdu_rx_payload_size(pdu);
    msg_field.dst = access_get_pdu_dst_addr(pdu);
    msg_field.src = access_get_pdu_src_addr(pdu);

    generic_level_server_t *server =GET_SERVER_MODEL_PTR(generic_level_server_t,model);
    server->tid_queue.inst_param.inst = (void *)server;

    LOG(3, "%s present_level:%x\n", __func__, server->msg_format.present_level);
    if(payload_size == sizeof(generic_move_msg_set_t))
    {
        generic_move_msg_set_t *p_pdu = (generic_move_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -1;
        }
        //this block code is used to  store valid message  within 6 seconds

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }
        server->server_state = GENERIC_TRANS_PROCESS;

        server->msg_format.target_level = 0x7fff;
        server->msg_format.params.move.required_move = p_pdu->move_level;
        *publish_status = (server->msg_format.target_level != server->msg_format.present_level);

        if(p_pdu->delay == 0 && p_pdu->trans_time == 0){
            level_server_move_action(server->delay_trans_timer);
        }else {//this block code is used to transition state


            server->delay_trans_timer->inst = server->tid_queue.inst_param.inst;
            server->delay_trans_timer->target_value = p_pdu->move_level;
            server->delay_trans_timer->trans_time = p_pdu->trans_time;
            server->delay_trans_timer->type = GENERIC_DELAY_TRANS_TIMER_MOVE;

            generic_deal_delay_trans_func(server->delay_trans_timer, p_pdu->delay, p_pdu->trans_time, level_server_move_action_then_publish);
        }

    }
    else if(payload_size == sizeof(generic_move_default_msg_set_t))
    {
        generic_move_default_msg_set_t *p_pdu = (generic_move_default_msg_set_t *)(access + 2);
        msg_field.tid = p_pdu->tid;

        if(generic_deal_recv_6s_tid_pkt(&server->tid_queue, &msg_field)) {
            return -2;
        }
        //this block code is used to  store valid message  within 6 seconds

        if(server->delay_trans_timer->Timer != NULL)
        {
            mesh_timer_stop(server->delay_trans_timer->Timer);
            mesh_timer_delete(server->delay_trans_timer->Timer);
            server->delay_trans_timer->Timer = NULL;
            server->delay_trans_timer->remain_tick_count = 0;
        }

        server->msg_format.target_level = 0x7fff;
        server->msg_format.params.move.required_move = p_pdu->move_level;
        *publish_status = (server->msg_format.target_level != server->msg_format.present_level);

        server->delay_trans_timer->trans_time = user_generic_default_transition_time_get(NULL);
        server->server_state = GENERIC_TRANS_PROCESS;

        if(server->delay_trans_timer->trans_time == 0)
            level_server_move_action(server->delay_trans_timer);
        else
            generic_deal_delay_trans_func(server->delay_trans_timer, 0, server->delay_trans_timer->trans_time, level_server_move_action_then_publish);
    }

    return 0;
}

void generic_move_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    int ret;
    LOG(3,"generic_move_set_rx\n");
    ret = generic_move_set_rx_handler(elmt,model,pdu, &publish_status);

    if(ret == 0)
        send_generic_level_status(elmt, model, pdu);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}
void generic_move_set_unacknowledged_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t publish_status = 0;
    LOG(3,"generic_move_set_unacknowledged_rx\n");
    generic_move_set_rx_handler(elmt,model,pdu, &publish_status);

    if(publish_status) {
        LOG(3, "%s:%d\n", __func__, __LINE__);
        model_status_publish();
    }
}



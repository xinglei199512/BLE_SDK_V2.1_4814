
#include "osapp_config.h"
#include "mesh_model.h"
#include "mesh_core_api.h"
#include "access_rx_process.h"
#include "foundation_common.h"
#include "foundation_msg_handler.h"
#include "config_server_events_api.h"
#include "mesh_kr_comm.h"
#include "mesh_iv_operation.h"
#include "access_tx_process.h"
#include "config_server.h"
#include "mesh_kr_server.h"
#include "node_save_common.h"
#include "mesh_sched.h"
#include "mesh_queued_msg.h"

static uint8_t composition_data_get_length(void)
{
    uint8_t len , i;
    mesh_elmt_t *elmt = get_mesh_elmt();
    len = 10;//CID,PID,VID,CRPL,Feature
    //every element in node
    for(i=0;i<get_element_num();++i)
    {
        len += 4;//LOC,NumS,NumV
        struct co_list_hdr *hdr = co_list_pick(&elmt[i].model_list);
        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if((model->sig_model) == true)
                len += 2;
            else
                len += 4;

            hdr = co_list_next(hdr);
        }
    }
    LOG(3,"composition_data_get_length=%d\n",len);
    return len;
}

static void composition_data_fill_buff(uint8_t * buf , uint8_t verify_length)
{
    uint16_t features = 0;
    uint8_t i,offset,numS,numV;
    mesh_elmt_t *elmt = get_mesh_elmt();
    mesh_core_params_t param1,param2,param3,param4;//get feature

    //CID,PID,VID,CRPL
    buf[0] = (MESH_PARAM_CID  >> 0) & 0xFF;
    buf[1] = (MESH_PARAM_CID  >> 8) & 0xFF;
    buf[2] = (MESH_PARAM_PID  >> 0) & 0xFF;
    buf[3] = (MESH_PARAM_PID  >> 8) & 0xFF;
    buf[4] = (MESH_PARAM_VID  >> 0) & 0xFF;
    buf[5] = (MESH_PARAM_VID  >> 8) & 0xFF;
    buf[6] = (MESH_PARAM_CRPL >> 0) & 0xFF;
    buf[7] = (MESH_PARAM_CRPL >> 8) & 0xFF;
    //features
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_RELAY     , &param1);
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_PROXY     , &param2);
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_FRIENT    , &param3);
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_LOW_POWER , &param4);
    features |= (param1.relay     == MESH_FEATURE_NOT_SUPPORT) ? 0 : 1<<0;
    features |= (param2.proxy     == MESH_FEATURE_NOT_SUPPORT) ? 0 : 1<<1;
    features |= (param3.friend    == MESH_FEATURE_NOT_SUPPORT) ? 0 : 1<<2;
    features |= (param4.low_power == MESH_FEATURE_NOT_SUPPORT) ? 0 : 1<<3;
    buf[8] = (features >> 0) & 0xFF;
    buf[9] = (features >> 8) & 0xFF;
    //Elements
    offset = 10;
    
    //every element in node
    for(i=0;i<get_element_num();++i)
    {
        //Calculate NumS and NumV
        numS = numV = 0;
        struct co_list_hdr *hdr = co_list_pick(&elmt[i].model_list);
        while(hdr)
        {
            //every model in element
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if((model->sig_model) == true ) numS ++;
            if((model->sig_model) == false) numV ++;
            hdr = co_list_next(hdr);
        }
        //Fill LOC,NumS,NumV
        buf[offset++] = (MESH_PARAM_LOC >> 0) & 0xFF;
        buf[offset++] = (MESH_PARAM_LOC >> 8) & 0xFF;
        buf[offset++] = numS;
        buf[offset++] = numV;
        //Fill SIG Models
        hdr = co_list_pick(&elmt[i].model_list);
        while(hdr)
        {
            //every model in element
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if((model->sig_model) == true)
            {
                buf[offset++] = (model->model_id >> 0) & 0xFF;
                buf[offset++] = (model->model_id >> 8) & 0xFF;
            }
            hdr = co_list_next(hdr);
        }
        //Fill Vendor Models
        hdr = co_list_pick(&elmt[i].model_list);
        while(hdr)
        {
            //every model in element
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if((model->sig_model) == false)
            {
                buf[offset++] = (model->model_id >> 0) & 0xFF;
                buf[offset++] = (model->model_id >> 8) & 0xFF;
                buf[offset++] = (model->model_id >> 16) & 0xFF;
                buf[offset++] = (model->model_id >> 24) & 0xFF;
            }
            hdr = co_list_next(hdr);
        }
    }    
    LOG(3,"verify_length=%d,offset = %d\n",verify_length,offset);
    BX_ASSERT(offset == verify_length);
}
static void config_composition_data_status_tx(model_base_t *model,uint16_t dst_addr,uint8_t page,void (*cb)(void *,uint8_t),uint16_t netkey_global_idx)
{
    uint8_t payload_size = composition_data_get_length();
    uint32_t opcode = 0;
    access_pdu_tx_t *ptr = NULL;
    uint8_t * buf = NULL;
    opcode = ONE_OCTET_OPCODE_GEN(FOUNDATION_MODELS_ONE_OCTET_OPCODE_OFFSET,Config_Composition_Data_Status);

    model_tx_msg_param_t model_tx_msg_param;
    memset(&model_tx_msg_param,0,sizeof(model_tx_msg_param_t));
    model_tx_msg_param.dst_addr.addr = dst_addr;
    model_tx_msg_param.src_addr = model->elmt->uni_addr;
    model_tx_msg_param.opcode = opcode;
    model_tx_msg_param.pdu_length = payload_size + 1;
    model_tx_msg_param.key.netkey_idx = netkey_global_idx;
    model_tx_msg_param.seg = true;
    ptr = access_model_msg_build(&model_tx_msg_param,cb);
    if(ptr == NULL) return;
    buf = access_model_tx_payload_ptr(ptr,opcode);
    buf[0] = page;
    composition_data_fill_buff(&buf[1] , payload_size);
    access_send(ptr);
    LOG(3,"config_composition_data_status_tx\n");
}

static void config_composition_data_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_composition_data_status_tx_done\n");
}

void config_composition_data_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    LOG(3,"config_composition_data_get_rx\n");
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    config_composition_data_status_tx(model , access_get_pdu_src_addr(pdu), 0 , config_composition_data_status_tx_done,netkey_global_idx);
}

static void config_beacon_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3, "config_beacon_status_tx_done\n");
}

extern uint8_t send_secoure_beacon;
void config_beacon_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    LOG(3, "config_beacon_get_rx\n");
    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu), &pdu->uppdu->rx_time, 
            config_beacon_status_tx_done, &send_secoure_beacon, 1, Config_Beacon_Status);
}

void config_beacon_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t beacon = *(uint8_t *)(access + 2);

    if(beacon > 1)
        return;
    
    send_secoure_beacon = beacon;
    LOG(3, "config_beacon_set_rx:%d\n", send_secoure_beacon);

    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu), &pdu->uppdu->rx_time, 
            config_beacon_status_tx_done, &send_secoure_beacon, 1, Config_Beacon_Status);
}
static void config_default_ttl_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3, "config_default_ttl_status_tx_done\n");
}

void config_default_ttl_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "config_default_ttl_get\n");
    uint8_t ttl = config_server_get_default_ttl();
    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu),
            &pdu->uppdu->rx_time, config_default_ttl_status_tx_done, &ttl, 1, Config_Default_TTL_Status);
}

void config_default_ttl_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t ttl = *(uint8_t *)(access + 2);

    if(ttl == 1 || ttl > 127)
        return;

    config_server_set_default_ttl(ttl);
    LOG(3, "config_default_ttl_set_rx:%x\n", ttl);
    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu),
            &pdu->uppdu->rx_time, config_default_ttl_status_tx_done, &ttl, 1, Config_Default_TTL_Status);
}

static void config_netkey_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_netkey_status_tx_done %d\n", status);
}
void handle_config_key_refresh_phase_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint32_t payload_size  = get_access_pdu_rx_payload_size(pdu);
    err_t status;
    config_msg_key_refresh_phase_status_t phase_status;
    if(payload_size != sizeof(config_msg_key_refresh_phase_get_t))
    {
        return;
    }
    config_msg_key_refresh_phase_get_t *kr_phase_get =(config_msg_key_refresh_phase_get_t *) (access_get_pdu_payload(pdu) + 2 );
    mesh_key_refresh_phase_t kr_phase = MESH_KEY_REFRESH_PHASE_0;
    status = mesh_kr_config_get_phase(kr_phase_get->netkey_index,&kr_phase);
     if(status != MESH_CORE_SUCCESS)
     {
         phase_status.status = Invalid_NetKey_Index;
     }
     else 
     {
        phase_status.status = Config_Success;
     }
     phase_status.netkey_index = kr_phase_get->netkey_index;
     phase_status.phase =  kr_phase;
     fd_model_two_octets_status_response_tx(pdu,config_netkey_status_tx_done,
        (uint8_t *)&phase_status,sizeof(config_msg_key_refresh_phase_status_t),Config_Key_Refresh_Phase_Status);
}

 void handle_config_key_refresh_phase_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint32_t payload_size  = get_access_pdu_rx_payload_size(pdu);
    err_t status;
    config_msg_key_refresh_phase_status_t phase_status;
    if(payload_size != sizeof(config_msg_key_refresh_phase_set_t))
    {
        return;
    }
    config_msg_key_refresh_phase_set_t *kr_phase_set =(config_msg_key_refresh_phase_set_t *) (access_get_pdu_payload(pdu) + 2 );
    mesh_key_refresh_transition_t trans =(mesh_key_refresh_transition_t) kr_phase_set->transition;
    status = mesh_kr_config_netkey_phase(kr_phase_set->netkey_index,trans);
    if(MESH_OK != status)
    {
        return;
    }
    mesh_key_refresh_phase_t kr_phase;
    status = mesh_kr_config_get_phase(kr_phase_set->netkey_index,&kr_phase);
    if(status != MESH_CORE_SUCCESS)
    {
        phase_status.status = Invalid_NetKey_Index;
    }
    else 
    {
        phase_status.status = Config_Success;
    }
    phase_status.netkey_index = kr_phase_set->netkey_index;
    phase_status.phase = kr_phase;
    fd_model_two_octets_status_response_tx(pdu,config_netkey_status_tx_done,
        (uint8_t *)&phase_status,sizeof(config_msg_key_refresh_phase_status_t),Config_Key_Refresh_Phase_Status);
    //user callback
    if(status == MESH_CORE_SUCCESS)
    {
        config_server_evt_param_t evt_param = {
        .key_refresh_phase_set.kr_phase = kr_phase,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_KEY_REFRESH_PHASE_SET , &evt_param);
    }
}
static void config_network_transmit_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_network_transmit_status_tx_done\n");
}

static void config_network_transmit_status_tx(uint8_t transmit, access_pdu_rx_t *pdu)
{
    ble_txrx_time_t rx_time = access_rx_get_rx_time(pdu);
    fd_model_two_octets_status_tx(access_get_pdu_netkey_global_index(pdu), access_get_pdu_src_addr(pdu), &rx_time, 
            config_network_transmit_status_tx_done, &transmit,1,Config_Network_Transmit_Status);

}

void config_network_transmit_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    mesh_core_params_t param;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    mesh_core_params_get(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param);

    config_network_transmit_status_tx((param.network_transmit.count & 0x7) | ((param.network_transmit.interval_steps & 0x1f) << 3),pdu);
}
static uint8_t netkey_identity = 1;
static void config_server_node_identity_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_server_node_identity_tx_done:%x\n", status);
}

void config_network_transmit_set_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    mesh_core_params_t param;
    uint8_t *access = access_get_pdu_payload(pdu);
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);

    param.network_transmit.count          = (*(uint8_t *)(access + 2)) & 0x7;
    param.network_transmit.interval_steps = ((*(uint8_t *)(access + 2)) >> 3) & 0x1f;
    mesh_core_params_set(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param);

    config_network_transmit_status_tx(*(uint8_t *)(access + 2), pdu);
}

void config_server_node_identity_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint16_t netkey_index = *(uint16_t *)(access + 2);

    dm_netkey_handle_t p_netkey_handle;
    uint8_t netkey_identity_temp = 0;
    uint8_t status = Config_Success;
    if( MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&p_netkey_handle))
    {
       status = Invalid_NetKey_Index;
       netkey_identity_temp = 0;
    }
    else
    {
        netkey_identity_temp = netkey_identity;
    }

    uint8_t resp[4];
    resp[0] = status;
    memcpy(&resp[1], access + 2, 2);
    resp[3] = netkey_identity_temp;
    LOG(3, "config_server_node_identity_get_rx:%x\n", netkey_identity);
    fd_model_two_octets_status_response_tx(pdu,config_server_node_identity_tx_done,
        resp, sizeof(resp), Config_Node_Identity_Status);
}

void config_server_node_identity_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t netkey_identity_temp = *(uint8_t *)(access + 4);
    uint16_t netkey_index = *(uint16_t *)(access + 2);
    dm_netkey_handle_t p_netkey_handle;
    uint8_t status;
    LOG(3, "config_server_node_identity_set_rx:%x %x\n", netkey_index, netkey_identity_temp);

    if(netkey_identity_temp >= 0x2)
        return;

    if( MESH_CORE_SUCCESS != dm_netkey_index_to_netkey_handle(netkey_index,&p_netkey_handle))
    {
        status = Invalid_NetKey_Index;
        netkey_identity_temp = 0;
    }
    else
    {
        netkey_identity = netkey_identity_temp;

        //        if(netkey_identity == 0)
        //            proxy_block_netkey_to_send_node_id(netkey_index);
        //        else
        //           proxy_unblock_netkey_to_send_node_id(netkey_index);
        status = Config_Success;
    }

    uint8_t resp[4];
    resp[0] = status;
    memcpy(&resp[1], access + 2, 2);
    resp[3] = netkey_identity_temp;
    fd_model_two_octets_status_response_tx(pdu,config_server_node_identity_tx_done,
            resp, sizeof(resp), Config_Node_Identity_Status);
}

static void scan_stop_then_delete_mesh_dir(void)
{
    node_delete_mesh_dir();
}

static void clear_flash_and_reset(void)
{
    //mesh_core_system_set(MESH_CORE_SYSTEM_ALL_RESET);
    mesh_sched_stop_scan(scan_stop_then_delete_mesh_dir);
}


void node_reset_start(void)
{
    LOG(3, "node_reset_start reset system\n");
    mesh_queued_msg_send((void (*)(void *))clear_flash_and_reset,NULL);
}


static void config_server_node_reset_tx_done(void *ptr,uint8_t status)
{
    node_reset_start();
    LOG(3,"config_server_node_reset_tx_done %d\n", status);
}

void config_server_node_reset_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    LOG(3,"config_server_node_reset_rx\n");
    fd_model_two_octets_status_response_tx(pdu,config_server_node_reset_tx_done,
        NULL,0,Config_Node_Reset_Status);
    //user callback
    
    config_server_evt_param_t evt_param = {
    .node_reset.reason = 0,
    };
    config_server_evt_act(CONFIG_SERVER_EVT_NODE_RESET , &evt_param);
}


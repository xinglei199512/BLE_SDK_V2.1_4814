
#include "osapp_config.h"
#include "mesh_model.h"
#include "mesh_core_api.h"
#include "access_rx_process.h"
#include "foundation_common.h"
#include "foundation_msg_handler.h"
#include "mesh_kr_comm.h"
#include "config_server_events_api.h"
#include "config_server_key.h"
#include "app_keys_dm.h"
#include "config_server.h"
#include "heartbeat.h"
#include "mesh_kr_server.h"

static void config_appkey_add_rx_aid_done(void* dm_handle)
{
    LOG(3,"config_appkey_add_rx_aid_done\n");
}

static void config_appkey_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_appkey_status_tx_done\n");
}

static void fetch_2_key_idx(uint8_t* buf_ptr , uint16_t *netkey_idx,uint16_t *appkey_idx)
{
    uint16_t tmp;
    tmp = (uint16_t)(buf_ptr[1]&0x0f) << 8;
    tmp += buf_ptr[0];
    *appkey_idx = tmp;
    tmp = (uint16_t)buf_ptr[2] << 4;
    tmp |= (buf_ptr[1]&0xf0) >> 4;
    *netkey_idx = tmp;
}

void config_appkey_add_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t netkey_idx;
    uint16_t appkey_idx;
    //dm_netkey_handle_t netkey_handle;
    dm_appkey_handle_t appkey_handle;
    err_t status;
    uint8_t * access = access_get_pdu_payload(pdu);
    fetch_2_key_idx(access+1,&appkey_idx,&netkey_idx);
    LOG(3,"netkey_idx=0x%x,appkey_idx=0x%x\n",netkey_idx,appkey_idx);
    
    status = dm_appkey_add(netkey_idx,appkey_idx,access+4,&appkey_handle,config_appkey_add_rx_aid_done);

    switch(status)
    {
        case MESH_DM_ERROR_INVALID_NETKEY_INDEX :
            status = Invalid_NetKey_Index;
            break;
        case MESH_DM_ERROR_INSUFFICIENT_RESOURCES :
            status = Insufficient_Resources;
            break;
        case MESH_DM_ERROR_KEY_INDEX_ALREADY_STORED : 
            {
                static int count = 0;
                if(count == 0)
                    status = Config_Success;
                else
                    status = Key_Index_Already_Stored;
                count++;
            }
            break;
        case MESH_DM_SUCCESS :
            status = Config_Success;
            break;
        default:
            status = Unspecified_Error;
            break;
    }

    if(netkey_idx == 1)
        status = Invalid_NetKey_Index;

    LOG(LOG_LVL_ERROR,"config_appkey_add_rx [status] = %d\n",status);

    uint8_t resp_buf[CONFIG_APPKEY_STATUS_PARAM_LENGTH];
    resp_buf[0] = status;
    memcpy(resp_buf + 1,access+1,3);
    fd_model_two_octets_status_response_tx(pdu,config_appkey_status_tx_done,
        resp_buf,CONFIG_APPKEY_STATUS_PARAM_LENGTH,Config_AppKey_Status);
    LOG(3,"config_appkey_add_rx->config_appkey_status_tx\n");
    //user callback
    if(status == Config_Success)
    {
        dm_appkey_handle_t appkey_handle;
        dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle);
        config_server_evt_param_t evt_param = {
        .appkey_add.appkey_handle = appkey_handle,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_APPKEY_ADD , &evt_param);
    }
}

void config_appkey_update_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t netkey_idx;
    uint16_t appkey_idx;
    dm_netkey_handle_t netkey_handle;
    dm_appkey_handle_t appkey_handle;
    mesh_key_refresh_phase_t phase;
    uint8_t * access = access_get_pdu_payload(pdu);
    fetch_2_key_idx(access + 1,&appkey_idx,&netkey_idx);
    LOG(3,"netkey_idx=0x%x,appkey_idx=0x%x\n",netkey_idx,appkey_idx);
    err_t status = Invalid_AppKey_Index;
    status = dm_netkey_index_to_netkey_handle(netkey_idx,&netkey_handle);
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    if(status != MESH_CORE_SUCCESS)
    {
        status = Invalid_NetKey_Index;
    }else
    {
        dm_netkey_kr_phase_get(netkey_handle,&phase);
        if(phase == MESH_KEY_REFRESH_PHASE_1)
        {
             if(MESH_CORE_SUCCESS !=dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle))
             {
                 status = Invalid_AppKey_Index;
             }
             else
             {
                 dm_appkey_update(appkey_handle,access+4,config_appkey_add_rx_aid_done);
                 status = Config_Success;
             }
        }
        else
        {
            status = Cannot_Update;
        }
    }

    if(appkey_idx == 1)
        status = Invalid_Binding;

    uint8_t resp_buf[CONFIG_APPKEY_STATUS_PARAM_LENGTH];
    resp_buf[0] = status;
    memcpy(resp_buf + 1,access+1,3);
    fd_model_two_octets_status_response_tx(pdu,config_appkey_status_tx_done,
        resp_buf,CONFIG_APPKEY_STATUS_PARAM_LENGTH,Config_AppKey_Status);
    LOG(3,"config_appkey_add_rx->config_appkey_status_tx\n");
    //user callback
    if(status == Config_Success)
    {
       config_server_evt_param_t evt_param = {
    .   appkey_update.appkey_handle = appkey_handle,
       };
       config_server_evt_act(CONFIG_SERVER_EVT_APPKEY_UPDATE , &evt_param);
    }
}

void config_appkey_delete_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t netkey_idx;
    uint16_t appkey_idx;
    dm_netkey_handle_t netkey_handle;
    dm_appkey_handle_t appkey_handle;
    uint8_t *access = access_get_pdu_payload(pdu);
    fetch_2_key_idx(access + 2, &netkey_idx, &appkey_idx);
    LOG(3,"netkey_idx=0x%x, appkey_idx=0x%x\n", netkey_idx, appkey_idx);
    err_t status = Invalid_AppKey_Index;
    status = dm_netkey_index_to_netkey_handle(netkey_idx,&netkey_handle);
    if(status != MESH_CORE_SUCCESS && status != MESH_CORE_ERROR_NOT_FOUND)
    {
        status = Invalid_NetKey_Index;
    }else
    {

        status = dm_appkey_index_to_appkey_handle(appkey_idx, &appkey_handle);
        if(status != MESH_CORE_SUCCESS)
        {
            status = dm_appkey_delete(appkey_handle);
            if(status == MESH_CORE_SUCCESS)
            {
                status = Config_Success;
            }else
            {
                status = Config_Success;
                //status = Invalid_AppKey_Index;
                //LOG(3,"Insufficient_Resources\n");
            }
        }
        else
        {
            status = Config_Success;
        }
    }
    uint8_t resp_buf[CONFIG_APPKEY_STATUS_PARAM_LENGTH];
    resp_buf[0] = status;
    memcpy(resp_buf + 1, access +2, 3);
    fd_model_two_octets_status_response_tx(pdu,config_appkey_status_tx_done,
            resp_buf,CONFIG_APPKEY_STATUS_PARAM_LENGTH,Config_AppKey_Status);
    LOG(3,"config_appkey_delete_rx->config_appkey_status_tx\n");
    //user callback

    if(status == Config_Success)
    {
        config_server_evt_param_t evt_param = {
            .appkey_delete.appkey_handle = appkey_handle,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_APPKEY_DELETE , &evt_param);
    }
}

void config_appkey_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint8_t *access = access_get_pdu_payload(pdu);
    uint16_t netkey_idx = *(uint16_t *)(access + 2);
    uint8_t resp[6];
    uint32_t count = 0;

    if((netkey_idx & 0xf000) || netkey_idx == 0xfff)
        resp[0] = Invalid_NetKey_Index;
    else {
        resp[0] = Config_Success;

        dm_get_all_appkey_index(resp + 3, &count);
    }
    memcpy(resp + 1, access + 2, 2);

    LOG(3, "config_appkey_get_rx:%d\n", count);

    fd_model_two_octets_status_response_tx(pdu, config_appkey_status_tx_done,
            resp, 3 + count, Config_AppKey_List);
}
static void config_model_app_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_model_app_status_tx_done\n");
}
void config_model_app_bind_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3,"config_model_app_bind_rx\n");
    //config_server_model_t *ptr = GET_SERVER_MODEL_PTR(config_server_model_t,model);
    uint16_t elmt_addr;
    uint16_t appkey_idx;
    uint16_t payload_size = get_access_pdu_rx_payload_size(pdu);
    uint8_t * access = access_get_pdu_payload(pdu);
    uint8_t buf[10];
    memcpy(&elmt_addr,&access[2], 2);
    memcpy(&appkey_idx,&access[4], 2);
    mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
    LOG(3,"elmt_addr:%x dst_elmt=0x%x payload_size:%d\n", elmt_addr, dst_elmt, payload_size);
    uint32_t model_id = 0;
    uint16_t tmp_model = 0;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    bool sig_model;
    model_base_t *dst_model = NULL;
    if(payload_size == 0x6)
    {
        sig_model = true;
        memcpy(&tmp_model, access_get_pdu_payload(pdu)+6, 2);
        model_id = tmp_model;
    }else if(payload_size == 0x8) {
        sig_model = false;
        memcpy(&model_id, access_get_pdu_payload(pdu) +6, 4);
    }else
    {
        LOG(LOG_LVL_WARN,"model app bind length error\n");
    }

    LOG(3,"model_id=0x%x sig_model:%d\n", model_id, sig_model);

    uint8_t status;

    do {
        if(!dst_elmt) {
            status = Invalid_Address;
            break;
        }

        if(appkey_idx & 0xf000) {
            status = Invalid_AppKey_Index;
            break;
        }

        dst_model = get_model_inst_from_elmt(dst_elmt, model_id, sig_model);
        LOG(3,"dst_model=0x%x\n", dst_model);
        if(dst_model)
        {
            LOG(3,"config_model_app_bind_rx:bind_appkey_to_model\n");
            bind_appkey_to_model(dst_model, appkey_idx, &status);
        }else
        {
            status = Invalid_Model;
            LOG(3,"config_model_app_bind_rx:Invalid_Model\n");
            break;
        }

        if(model_id == 0) {
            status = Cannot_Bind;
            break;
        }
    }while(0);

    //status = Config_Success;
    LOG(3,"config_model_app_status_tx status:%x rx_time:%x\n", status, pdu->uppdu->rx_time.time_cnt);
    buf[0] = status;
    memcpy(&buf[1],access_get_pdu_payload(pdu) +2,sig_model?6:8);
    fd_model_two_octets_status_response_tx(pdu,config_model_app_status_tx_done,buf,sig_model?7:9,Config_Model_App_Status);
    //user callback

    if(status == Config_Success)
    {
        dm_appkey_handle_t appkey_handle;
        dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle);
        config_server_evt_param_t evt_param = {
            .model_app_bind.model = dst_model,
            .model_app_bind.appkey_handle = appkey_handle,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_MODEL_APP_BIND, &evt_param);
    }
}

void config_model_app_unbind_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3,"config_model_app_unbind_rx\n");
    //config_server_model_t *ptr = GET_SERVER_MODEL_PTR(config_server_model_t,model);
    uint16_t elmt_addr;
    uint16_t appkey_idx;
    uint16_t payload_size = get_access_pdu_rx_payload_size(pdu);
    uint8_t * access = access_get_pdu_payload(pdu);
    uint8_t buf[10];
    memcpy(&elmt_addr, access+2, 2);
    memcpy(&appkey_idx, access+4, 2);
    mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
    LOG(3,"dst_elmt=0x%x\n",dst_elmt);
    uint32_t model_id = 0;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    bool sig_model;
    model_base_t *dst_model = NULL;
    if(payload_size == 0x6)
    {
        sig_model = true;
        memcpy(&model_id, access + 6, 2);
    }else if(payload_size == 0x8) {
        sig_model = false;
        memcpy(&model_id, access + 6, 4);
    }else
    {
        LOG(LOG_LVL_WARN,"model app unbind length error\n");
    }

    LOG(3,"model_id=0x%x\n", model_id);
    uint8_t status;
    do {
        if(!dst_elmt) {
            status = Invalid_Address;
            break;
        }

        if(appkey_idx & 0xf000) {
            status = Invalid_AppKey_Index;
            break;
        }

        dst_model = get_model_inst_from_elmt(dst_elmt, model_id, sig_model);
        LOG(3,"dst_model=0x%x\n", dst_model);
        if(dst_model)
        {
            LOG(3,"config_model_app_bind_rx:bind_appkey_to_model\n");
            unbind_appkey_to_model(dst_model, appkey_idx, &status);
        }else
        {
            status = Invalid_Model;
            LOG(3,"config_model_app_bind_rx:Invalid_Model\n");
        }
    }while(0);
    buf[0] = status;
    memcpy(&buf[1],access_get_pdu_payload(pdu) +2,sig_model?6:8);
    fd_model_two_octets_status_response_tx(pdu,config_model_app_status_tx_done,buf,sig_model?7:9,Config_Model_App_Status);
    
    LOG(3,"config_model_app_status_tx\n");
    //user callback
    
    if(status == Config_Success)
    {
        dm_appkey_handle_t appkey_handle;
        dm_appkey_index_to_appkey_handle(appkey_idx, &appkey_handle);
        config_server_evt_param_t evt_param = {
        .model_app_unbind.model = dst_model,
        .model_app_unbind.appkey_handle = appkey_handle,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_MODEL_APP_UNBIND, &evt_param);
    }

}
static void config_netkey_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_netkey_status_tx_done %d\n", status);
}

static void config_network_keys_done_cb(void* handle)
{
    LOG(3, "config_network_keys_done_cb\n");
}
void config_netkey_add_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    config_msg_netkey_status_t netkey_status;
    dm_netkey_handle_t netkey_handle;
    //dm_appkey_handle_t appkey_handle;
    err_t status;
    uint8_t *access = access_get_pdu_payload(pdu);
    config_msg_netkey_add_update_t *kr_netkey_set =(config_msg_netkey_add_update_t *) (access + 2 );

    status = dm_netkey_add(kr_netkey_set->netkey_index, kr_netkey_set->netkey, &netkey_handle, config_network_keys_done_cb);

    switch(status)
    {
        case MESH_DM_ERROR_INVALID_NETKEY_INDEX :
            netkey_status.status = Invalid_NetKey_Index;
            break;
        case MESH_DM_ERROR_INSUFFICIENT_RESOURCES :
            netkey_status.status = Insufficient_Resources;
            break;
        case MESH_DM_ERROR_KEY_INDEX_ALREADY_STORED : 
            {
#if 0
                static uint8_t count = 0;
                if(count == 0)
                    netkey_status.status = Config_Success;
                else
                    netkey_status.status = Key_Index_Already_Stored;
                count++;

                if(kr_netkey_set->netkey_index == 1)
#endif
                    netkey_status.status = Config_Success;
            }
            break;
        case MESH_DM_SUCCESS :
            netkey_status.status = Config_Success;
            break;
        default:
            netkey_status.status = Key_Index_Already_Stored;
            break;
    }

    LOG(LOG_LVL_ERROR,"config_netkey_add_rx [status] = %x %x netkey_index:%x\n", netkey_status.status, status, kr_netkey_set->netkey_index);

    if(kr_netkey_set->netkey_index == 2)
        netkey_status.status = Insufficient_Resources;

    netkey_status.netkey_index = kr_netkey_set->netkey_index;
    fd_model_two_octets_status_response_tx(pdu, config_netkey_status_tx_done, 
        (uint8_t *)&netkey_status, sizeof(config_msg_netkey_status_t), Config_NetKey_Status);
    //user callback
    if(status == MESH_CORE_SUCCESS)
    {
        dm_netkey_handle_t netkey_handle;
        dm_netkey_index_to_netkey_handle(kr_netkey_set->netkey_index,&netkey_handle);
        config_server_evt_param_t evt_param = {
            .netkey_update.netkey_handle = netkey_handle,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_NETKEY_UPDATE , &evt_param);
    }
//   extern  void proxy_block_netkey_to_send_node_id(uint16_t netkey_index);
//   proxy_block_netkey_to_send_node_id(kr_netkey_set->netkey_index);
}

void config_netkey_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t netkey_idx[6] = {0};
    uint32_t count;
    dm_get_all_netkey_idx(netkey_idx, &count);

    LOG(3, "config_netkey_get_rx:%d\n", count);

    fd_model_two_octets_status_response_tx(pdu, config_netkey_status_tx_done, 
        (uint8_t *)netkey_idx, count, Config_NetKey_List);
}

void config_netkey_delete_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    LOG(3, "config_netkey_delete_rx\n");
    config_msg_netkey_status_t netkey_status;
    dm_netkey_handle_t netkey_handle;
    //dm_appkey_handle_t appkey_handle;
    err_t status;
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t count = 3;
#if 0
    uint8_t netkey_idx[6] = {0};

    dm_get_all_netkey_idx(netkey_idx, &count);
#endif

    config_msg_netkey_delete_t *kr_netkey_delete =(config_msg_netkey_delete_t *) (access + 2 );

    dm_netkey_index_to_netkey_handle(kr_netkey_delete->netkey_index, &netkey_handle);
#if 0
    if(kr_netkey_delete->netkey_index == 1)
        count = 2;
#endif

    if(count > 2) {
        status = dm_netkey_delete(netkey_handle);
        if(status == MESH_CORE_SUCCESS)
        {
            netkey_status.status = Config_Success;
        }else
        {
            netkey_status.status = Invalid_NetKey_Index;
            //netkey_status.status = Config_Success;
            LOG(3,"Insufficient_Resources\n");
        }
        stop_heartbeat_massage();
    }else
        netkey_status.status = Cannot_Remove;


    LOG(LOG_LVL_ERROR,"config_netkey_delete_rx status:%x netkey_index:%x %x\n", status, kr_netkey_delete->netkey_index, kr_netkey_delete->netkey_index);

#if 0
    if(netkey_status.status == Cannot_Remove)
        netkey_status.netkey_index = 1;
    else
#endif
        netkey_status.netkey_index = kr_netkey_delete->netkey_index;

    fd_model_two_octets_status_response_tx(pdu, config_netkey_status_tx_done, 
        (uint8_t *)&netkey_status, sizeof(config_msg_netkey_status_t), Config_NetKey_Status);
    //user callback
    if(status == MESH_CORE_SUCCESS)
    {
        dm_netkey_handle_t netkey_handle;
        dm_netkey_index_to_netkey_handle(kr_netkey_delete->netkey_index,&netkey_handle);
        config_server_evt_param_t evt_param = {
            .netkey_update.netkey_handle = netkey_handle,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_NETKEY_UPDATE , &evt_param);
    }

}

void config_server_update_netkey_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
     uint32_t payload_size  = get_access_pdu_rx_payload_size(pdu);
     err_t status;
     config_msg_netkey_status_t netkey_status;
     uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
     if(payload_size != sizeof(config_msg_netkey_add_update_t))
     {
         return;
     }
     LOG(3,"config_server_update_netkey\n");
     config_msg_netkey_add_update_t *kr_netkey_set =(config_msg_netkey_add_update_t *) (access_get_pdu_payload(pdu) + 2 );

     status = mesh_kr_config_netkey_update(kr_netkey_set->netkey_index,kr_netkey_set->netkey);
     if(status ==MESH_CORE_ERROR_FORBIDDEN)
     {
        netkey_status.status =  Cannot_Update;
     }
     else if(status != MESH_CORE_SUCCESS)
     {
         netkey_status.status = Invalid_NetKey_Index;
     }
     else 
     {
        netkey_status.status = Config_Success;
     }
     netkey_status.netkey_index = kr_netkey_set->netkey_index;
#if 0
     if(kr_netkey_set->netkey_index == 0xfff)
         netkey_status.netkey_index = 0;
#endif
     if(kr_netkey_set->netkey_index == 0x1) {
         netkey_status.status = Invalid_NetKey_Index;
         netkey_status.netkey_index = 2;
     }

    fd_model_two_octets_status_response_tx(pdu,config_netkey_status_tx_done,
        (uint8_t *)&netkey_status,sizeof(config_msg_netkey_status_t),Config_NetKey_Status);
     //user callback
     if(status == MESH_CORE_SUCCESS)
     {
         dm_netkey_handle_t netkey_handle;
         dm_netkey_index_to_netkey_handle(kr_netkey_set->netkey_index,&netkey_handle);
         config_server_evt_param_t evt_param = {
         .netkey_update.netkey_handle = netkey_handle,
         };
         config_server_evt_act(CONFIG_SERVER_EVT_NETKEY_UPDATE , &evt_param);
     }
}
static void config_model_app_list_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_model_app_list_tx_done\n");
}

void config_vendor_model_app_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint16_t elmt_addr;
    uint32_t model_id;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t status = Config_Success;
    model_base_t *dst_model = NULL;

    elmt_addr = *(uint16_t *)(access + 2);
    model_id = (*(uint32_t *)(access + 4));

    LOG(3, "config_vendor_model_app_get_rx elmt_addr:%x model_id:%x\n", elmt_addr, model_id);
    do
    {
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            break;
        }

        dst_model = get_model_inst_from_elmt(dst_elmt, model_id, false);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            break;
        }
    }while (0);

    uint8_t resp_buf[13];
    uint8_t count = 0;
    resp_buf[0] = status;
    memcpy(&resp_buf[1], access + 2, 6);

    for(int i = 0; i < dst_model->subscription_list_size; i++) {
        LOG(3, "%s save_bound_key:\n", __func__);
        if(dst_model->bound_key_buf[i] != NULL)
            memcpy(&resp_buf[7 + count++ * 2], &dst_model->bound_key_buf[i]->global_idx, 2);
    }

    fd_model_two_octets_status_response_tx(pdu, config_model_app_list_tx_done, 
        resp_buf, 7 + count * 2, Config_Vendor_Model_App_List);
}
void config_SIG_model_app_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint16_t elmt_addr;
    uint32_t model_id;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t status = Config_Success;
    model_base_t *dst_model = NULL;

    elmt_addr = *(uint16_t *)(access + 2);
    model_id = *(uint16_t *)(access + 4);

    do
    {
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            break;
        }

        dst_model = get_model_inst_from_elmt(dst_elmt, model_id, true);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            break;
        }
    }while (0);

    LOG(3, "config_SIG_model_app_get_rx elmt_addr:%x model_id:%x status:%d dst_model:%x %p\n", elmt_addr, model_id, status, dst_model, dst_model);
    uint8_t resp_buf[11];
    uint8_t count = 0;
    resp_buf[0] = status;
    memcpy(&resp_buf[1], access + 2, 4);

    for(int i = 0; i < dst_model->bound_key_buf_size; i++) {
        LOG(3, "%s save_bound_key:\n", __func__);
        if(dst_model->bound_key_buf[i] != NULL)
            memcpy(&resp_buf[5 + count++ * 2], &dst_model->bound_key_buf[i]->global_idx, 2);
    }
    fd_model_two_octets_status_response_tx(pdu, config_model_app_list_tx_done, 
    resp_buf, 5 + count * 2, Config_SIG_Model_App_List);
}


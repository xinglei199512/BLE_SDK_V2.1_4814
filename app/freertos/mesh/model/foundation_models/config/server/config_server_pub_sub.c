
#include "osapp_config.h"
#include "mesh_model.h"
#include "mesh_core_api.h"
#include "access_rx_process.h"
#include "foundation_common.h"
#include "foundation_msg_handler.h"
#include "config_server_events_api.h"
#include "config_server_pub_sub.h"
#include "app_keys_dm.h"
#include "node_save_model.h"
#include "model_publish.h"
#include "config_server.h"
#include "node_save_model.h"


void config_model_publication_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_model_publication_status_tx_done\n");

}

void config_model_publication_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint32_t model_id = 0;
    uint16_t elmt_addr;
    uint16_t publish_addr;
    uint16_t appkey_idx;
    uint8_t credential_flag;
    model_base_t *dst_model;
    dm_appkey_handle_t appkey_handle;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t * access = access_get_pdu_payload(pdu);
    memcpy(&elmt_addr, access + 1, 2);
    bool sig_model = (get_access_pdu_rx_payload_size(pdu)) == 11 ? true : false;
    memcpy(&model_id, access + 10, sig_model ? sizeof(uint16_t) : sizeof(uint32_t));
    uint8_t status;
    do
    {
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            LOG(3,"config_model_publication_status_tx_Invalid_Address\n");
            break;
        }
        dst_model = get_model_inst_from_elmt(dst_elmt,model_id,sig_model);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            LOG(3,"config_model_publication_status_tx_Invalid_Model\n");
            break;
        }
        if(dst_model->publish == NULL)
        {
            status = Invalid_Publish_Parameters;
            LOG(3,"config_model_publication_status_tx_Invalid_Publish_Parameters\n");
            break;
        }
        memcpy(&appkey_idx,access + 5,sizeof(uint16_t));
        appkey_idx &= 0xfff;
        if(dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle)!=MESH_CORE_SUCCESS)
        {
        
            LOG(3,"config_model_publication_status_tx_Invalid_AppKey_Index\n");
            status = Invalid_AppKey_Index;
            break;
        }
        dst_model->publish->appkey_idx = appkey_idx;
        dst_model->publish->addr.is_virt = false;
        memcpy(&publish_addr,access+3,2);
        dst_model->publish->addr.addr.addr = publish_addr;
        credential_flag = access[6]>>4 & 0x1;
        dst_model->publish->credential_flag = credential_flag;
        dst_model->publish->rfu = 0;
        dst_model->publish->ttl = access[7];
#if 0
        if(dst_model->publish->ttl == MESH_PUBLISH_DEFAULY_TTL_FLAG)
        {
            dst_model->publish->ttl = ACCESS_DEFAULT_TTL;
        }
#endif
        dst_model->publish->period = *(publish_period_t *)(access+8);
        dst_model->publish->retransmit = *(transmit_state_t *)(access+9);      
        LOG(3,"config_model_publication_status_tx_Config_Success\n");
        status = Config_Success;
        if( IS_UNASSIGNED_ADDR(publish_addr))
        {
             node_delete_publish_list(dst_model,0);
        }
        else
        {
           node_save_publish_list(dst_model,0);
        }
        model_publish_period_set(dst_model->publish,dst_model->publish->publish_period_cb,(void *)dst_model);
    }while (0);
    uint8_t resp_buf[14];

    if(status != Config_Success || publish_addr == 0)
    {
        memset(&resp_buf[3],0,7);
        memset(dst_model->publish, 0, sizeof(model_publish_state_t));
    }else
    {
        memcpy(&resp_buf[3], access +3, 7);
    }

    resp_buf[0] = status;
    memcpy(&resp_buf[1],&elmt_addr,sizeof(uint16_t));
    memcpy(&resp_buf[10],&model_id,sig_model ? sizeof(uint16_t) : sizeof(uint32_t));

    LOG(3, "%s:%x %x %x \n", __func__, resp_buf[5], resp_buf[6], resp_buf[7]);
    resp_buf[6] &= 0x1f;
    fd_model_two_octets_status_response_tx(pdu,config_model_publication_status_tx_done,resp_buf,sig_model ? 12 : 14,Config_Model_Publication_Status);
    //user callback
    
    if(status == Config_Success)
    {
        config_server_evt_param_t evt_param = {
        .model_publication_set.model = dst_model,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_MODEL_PUBLICATION_SET,&evt_param);
    }
}


void config_model_publication_get_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint32_t model_id = 0;
    uint16_t elmt_addr;
    uint8_t resp_buf[14];
    bool sig_model = get_access_pdu_rx_payload_size(pdu) == 4 ? true : false;
    uint8_t * access = access_get_pdu_payload(pdu);
    memcpy(&elmt_addr, access + 2, 2);
    memcpy(&model_id, access + 4, sig_model ? sizeof(uint16_t) : sizeof(uint32_t));

    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t status;
    do
    {
        memset(&resp_buf[3],0,7);
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            LOG(3,"config_model_publication_status_tx_Invalid_Address\n");
            break;
        }
        model_base_t *dst_model = get_model_inst_from_elmt(dst_elmt,model_id,sig_model);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            LOG(3,"config_model_publication_status_tx_Invalid_Model\n");
            break;
        }
        if(dst_model->publish == NULL)
        {
            status = Invalid_Publish_Parameters;
            LOG(3,"config_model_publication_status_tx_Invalid_Publish_Parameters\n");
            break;
        }
        config_model_publication_status_t *p_status =(config_model_publication_status_t *) resp_buf;

        p_status->state.appkey_index = dst_model->publish->appkey_idx;

        //if(dst_model->publish->addr.addr.addr & 0xC000 == 0x8000)
            p_status->publish_address = dst_model->publish->addr.addr.addr;
        //else
        //    p_status->publish_address = 0;

        p_status->state.credential_flag = dst_model->publish->credential_flag;
#if 1
        p_status->state.publish_ttl = dst_model->publish->ttl;
#else
        if(dst_model->publish->ttl == MESH_PUBLISH_DEFAULY_TTL_FLAG)
        {
            dst_model->publish->ttl = get_default_ttl();
        }
#endif
        p_status->state.rfu = 0;
        p_status->state.num_steps = dst_model->publish->period.num_steps;
        p_status->state.step_resolution = dst_model->publish->period.step_resolution;
        p_status->state.retransmit_count = dst_model->publish->retransmit.count;
        p_status->state.retransmit_interval = dst_model->publish->retransmit.interval_steps;
        
        LOG(3,"config_model_publication_status_tx_Config_Success addr:%x ttl:%x\n", p_status->publish_address, dst_model->publish->ttl);
        status = Config_Success;
    }while (0);
    resp_buf[0] = status;
    memcpy(&resp_buf[1],&elmt_addr,sizeof(uint16_t));
    memcpy(&resp_buf[10],&model_id,sig_model ? sizeof(uint16_t) : sizeof(uint32_t));
    LOG(3, "%s:%x %x %x %x\n", __func__, resp_buf[6], model_id, elmt_addr, sig_model);
    resp_buf[6] &= 0x1f;
    fd_model_two_octets_status_response_tx(pdu,config_model_publication_status_tx_done,resp_buf,sig_model ? 12 : 14,Config_Model_Publication_Status);
    //user callback
    //config_server_evt_act(CONFIG_SERVER_EVT_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET,elmt , model , pdu);
}

void config_model_publication_vir_set_rx_cb(model_base_t *model, access_pdu_param_t *access_param, virt_addr_mngt_t *p_addr)
{
    dm_appkey_handle_t appkey_handle;
    model_base_t *dst_model;

    uint8_t status;
    do
    {
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(access_param->elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            LOG(3,"config_model_publication_status_tx_Invalid_Address\n");
            break;
        }
        dst_model = get_model_inst_from_elmt(dst_elmt, access_param->model_id, access_param->sig_model);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            LOG(3,"config_model_publication_status_tx_Invalid_Model\n");
            break;
        }
        if(dst_model->publish == NULL)
        {
            status = Invalid_Publish_Parameters;
            LOG(3,"config_model_publication_status_tx_Invalid_Publish_Parameters\n");
            break;
        }
        if(dm_appkey_index_to_appkey_handle(access_param->appkey_idx, &appkey_handle) != MESH_CORE_SUCCESS)
        {
        
            LOG(3,"config_model_publication_status_tx_Invalid_AppKey_Index\n");
            status = Invalid_AppKey_Index;
            break;
        }
        dst_model->publish->appkey_idx = access_param->appkey_idx;
        dst_model->publish->addr.is_virt = true;
        dst_model->publish->addr.addr.virt = p_addr;
        if(Config_Success != status)
        {
            LOG(3,"virt_addr_add fail \n");
            break;
        }
        dst_model->publish->credential_flag = access_param->credential_flag;
        dst_model->publish->ttl = access_param->ttl;
        dst_model->publish->rfu = 0;
        dst_model->publish->period = *(publish_period_t *)&(access_param->period);
        dst_model->publish->retransmit = *(transmit_state_t *)&(access_param->retransmit);      
        status = Config_Success;
        if( IS_UNASSIGNED_ADDR(dst_model->publish->addr.addr.virt->virt_addr))
        {
             node_delete_publish_list(dst_model,0);
        }
        else
        {
           node_save_publish_list(dst_model,0);
        }
        model_publish_period_set(dst_model->publish, dst_model->publish->publish_period_cb, (void *)dst_model);
    }while (0);

    uint8_t resp_buf[14];
    resp_buf[0] = status;
    memcpy(&resp_buf[1], &access_param->elmt_addr, sizeof(uint16_t));
    memcpy(&resp_buf[10], &access_param->model_id, access_param->sig_model ? sizeof(uint16_t) : sizeof(uint32_t));
    if(status != Config_Success)
    {
        memset(&resp_buf[3],0,7);
    }else
    {
        memcpy(&resp_buf[3], &p_addr->virt_addr, 2);
        resp_buf[5] = access_param->appkey_idx & 0xff;
        resp_buf[6] = 0;
        resp_buf[6] = ((access_param->appkey_idx >> 8) & 0xf) | ((access_param->credential_flag << 4) & 0x10);
        resp_buf[7] = access_param->ttl;
        resp_buf[8] = access_param->period;
        resp_buf[9] = access_param->retransmit;
    } 
    LOG(3, "config_model_publication_vir_set_rx_cb value:%x %x %x\n", access_param->ttl, access_param->period, access_param->retransmit);
    fd_model_two_octets_status_tx(access_param->netkey_global_idx, access_param->src_addr,&access_param->rx_time, config_model_publication_status_tx_done,resp_buf,access_param->sig_model ? 12 : 14,Config_Model_Publication_Status);
    //user callback
    if(status == Config_Success)
    {
        config_server_evt_param_t evt_param = {
        .model_publication_set.model = dst_model,
        };
        config_server_evt_act(CONFIG_SERVER_EVT_MODEL_PUBLICATION_VIRTUAL_ADDRESS_SET, &evt_param);
    }
}


extern void save_virt_addr_param(model_base_t *model, access_pdu_param_t *access_param, void (*cb)(model_base_t *, access_pdu_param_t *, virt_addr_mngt_t *));
void config_model_publication_vir_set_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint32_t model_id = 0;
    uint16_t elmt_addr;
    uint16_t appkey_idx;
    uint8_t credential_flag;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t * access = access_get_pdu_payload(pdu);
    bool sig_model = get_access_pdu_rx_payload_size(pdu) == 25 ? true : false;
    access_pdu_param_t access_param;
    uint8_t subscribe_addr[LABEL_UUID_SIZE];
    uint8_t status;

    memcpy(&elmt_addr, access + 2, 2);
    memcpy(subscribe_addr, access + 4, 16);
    memcpy(&appkey_idx, access + 20, sizeof(uint16_t));
    appkey_idx &= 0xfff;
    memcpy(&model_id, access + 25, sig_model ? sizeof(uint16_t) : sizeof(uint32_t));

    credential_flag = access[21]>>4 & 0x1;
    access_param.model_id = model_id;
    access_param.elmt_addr = elmt_addr;
    access_param.netkey_global_idx = netkey_global_idx;
    access_param.src_addr = access_get_pdu_src_addr(pdu);
    access_param.sig_model = sig_model;
    access_param.appkey_idx = appkey_idx;
    access_param.credential_flag = credential_flag;
    access_param.rfu = 0;
    access_param.ttl = access[22];
    access_param.period = access[23];
    access_param.retransmit = access[24];
    access_param.rx_time =   access_rx_get_rx_time(pdu);
    LOG(3, "config_model_publication_vir_set_rx value:%x %x %x access:%x %x %x %x\n", access_param.ttl, access_param.period, access_param.retransmit, access[21], access[22], access[23], access[24]);

    save_virt_addr_param(model, &access_param, config_model_publication_vir_set_rx_cb);
    virt_addr_add(subscribe_addr, &status);
}

uint8_t config_model_subscription_add(mesh_elmt_t *elmt,model_base_t *model,uint16_t addr)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    
    for(i=0;i<model->subscription_list_size;++i)
    {
        if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) && model->subscription_list[i].addr.addr == addr)
        {
            i = model->subscription_list_size + 1;
            break;
        }
    }
    if(i <= model->subscription_list_size)
    {
        for(i=0;i<model->subscription_list_size;++i)
        {
            if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) == false)
            {
                model->subscription_list[i].addr.addr = addr;
                model->subscription_list[i].is_virt = false;
                node_save_subscription_list(model , i);
                break;
            }
        }
    }

    if(i == model->subscription_list_size + 1) {
        return Invalid_Address;
    }else if(i == model->subscription_list_size) {
        return Storage_Failure;
    }else {
        return Config_Success;
    }
}

uint8_t config_model_subscription_delete(mesh_elmt_t *elmt,model_base_t *model,uint16_t addr)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    for(i=0; i<model->subscription_list_size; ++i)
    {
        //LOG(3, "config_model_subscription_delete addr:%x %x\n", model->subscription_list[i].addr.addr, addr);
        if(model->subscription_list[i].addr.addr == addr)
        {
            model->subscription_list[i].addr.addr = 0;
            model->subscription_list[i].is_virt = false;
            node_delete_subscription_list(model , i);
            break;
        }
    }
    if(i == model->subscription_list_size) { 
        return Invalid_Address;
    }else {
        return Config_Success;
    }
}

uint8_t config_model_subscription_delete_all(mesh_elmt_t *elmt,model_base_t *model)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    for(i=0;i<model->subscription_list_size;++i)
    {
       // LOG(3, "config_model_subscription_delete_all addr:%x %x\n", model->subscription_list[i].addr.addr, IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]));
        if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]))
        {
            if(model->subscription_list[i].is_virt)
            {
                free_virt_addr_buf(model->subscription_list[i].addr.virt);
            }
            model->subscription_list[i].addr.addr = 0;
            model->subscription_list[i].is_virt = false;
            node_delete_subscription_list(model , i);
        }
    }
    return Config_Success;
}


uint8_t config_model_subscription_overwrite(mesh_elmt_t *elmt,model_base_t *model,uint16_t addr)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    for(i=0;i<model->subscription_list_size;++i)
    {
        
		if(!(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) == false))
		{
			model->subscription_list[i].addr.addr = 0;
			model->subscription_list[i].is_virt = false;
			node_delete_subscription_list(model , i);
			break;
		}
    }
    for(i=0;i<model->subscription_list_size;++i)
    {
        if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) == false)
        {
            model->subscription_list[i].addr.addr = addr;
            model->subscription_list[i].is_virt = false;
            node_save_subscription_list(model , i);
            break;
        }
    }
    if(i == model->subscription_list_size)
    {
        return Insufficient_Resources;
    }else
    {
        return Config_Success;
    }
}
static void config_model_subscription_status_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_model_subscription_status_tx_done\n");
}


void config_model_subscription_update_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t elmt_addr;
    uint16_t subscribe_addr;
    uint8_t * access = access_get_pdu_payload(pdu);
    memcpy(&elmt_addr,access + 2,2);
    memcpy(&subscribe_addr,access + 4,2);
	
    bool sig_model;
    uint32_t model_id;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    if(get_access_pdu_rx_payload_size(pdu) == 8)
    {
        sig_model = false;
        memcpy(&model_id,access + 6,4);
    }else
    {
        sig_model = true;
        model_id= access[6] | (access[7]<<8);
    }
    uint16_t opcode = (access[0] << 8) | access[1];
    uint8_t status;
    do
    {
        if((subscribe_addr & 0xc000) != 0xc000) {
            status = Invalid_Address;
            break;
        }
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            break;
        }
        model_base_t *dst_model = get_model_inst_from_elmt(dst_elmt, model_id, sig_model);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            break;
        }
        switch(opcode & 0x3fff)
        {
            case Config_Model_Subscription_Add:
                status = config_model_subscription_add(dst_elmt,dst_model,subscribe_addr);
                if(status != Config_Success)
                    LOG(3, "status:%x\n", status);
                status = Config_Success;
                //user callback
                if(status == Config_Success)
                {
                      config_server_evt_param_t evt_param = {
                     .model_subscription_add.model = model,
                     .model_subscription_add.address = subscribe_addr,
                     };
                    config_server_evt_act(CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_ADD,&evt_param);
                }
                break;
            case Config_Model_Subscription_Delete:
                 status = config_model_subscription_delete(dst_elmt,dst_model,subscribe_addr);
                //user callback
                if(status == Config_Success)
                {
                     config_server_evt_param_t evt_param = {
                    .model_subscription_delete.model = model,
                    .model_subscription_delete.address = subscribe_addr,
                    };
                    config_server_evt_act(CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_DELETE,&evt_param);
                }
                break;
            case Config_Model_Subscription_Overwrite:
                status = config_model_subscription_overwrite(dst_elmt,dst_model,subscribe_addr);
                //user callback
                 if(status == Config_Success)
                 {
                     config_server_evt_param_t evt_param = {
                     .model_subscription_overwrite.model = model,
                     .model_subscription_overwrite.address = subscribe_addr,
                     };
                    config_server_evt_act(CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_OVERWRITE,&evt_param);
                }
                break;
            default:
                BX_ASSERT(0);
                break;
        }
    }while (0);
    uint8_t resp_buf[9] = {0};
    resp_buf[0] = status;
    if(status == 0)
        memcpy(&resp_buf[1], access + 2, sig_model ? 6 : 8);
    else
        memcpy(&resp_buf[1], access + 2, 4);
    fd_model_two_octets_status_response_tx(pdu,config_model_subscription_status_tx_done,resp_buf,sig_model?7:9,Config_Model_Subscription_Status);
}

void config_model_subscription_delete_all_rx(mesh_elmt_t *elmt,model_base_t *model,access_pdu_rx_t *pdu)
{
    uint16_t elmt_addr;
    bool sig_model;
    uint32_t model_id;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t * access = access_get_pdu_payload(pdu);
    memcpy(&elmt_addr,access + 2,2);
    if(get_access_pdu_rx_payload_size(pdu) == 8)
    {
        sig_model = false;
        memcpy(&model_id,access + 4,4);
    }else
    {
        sig_model = true;
        model_id= access[4] | (access[5]<<8);
    }
    uint8_t status;
    do
    {
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            break;
        }
        model_base_t *dst_model = get_model_inst_from_elmt(dst_elmt, model_id, sig_model);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            break;
        }
        status = config_model_subscription_delete_all(dst_elmt,dst_model);
    }while (0);
    uint8_t resp_buf[9];
    memset(resp_buf, 0,9);
    resp_buf[0] = status;
    memcpy(&resp_buf[1],access + 2,2);
    memcpy(&resp_buf[5],access +2,sig_model ? 2 : 4);
    fd_model_two_octets_status_response_tx(pdu,config_model_subscription_status_tx_done,resp_buf,sig_model?7:9,Config_Model_Subscription_Status);
    config_server_evt_param_t evt_param = {
    .model_subscription_delete_all.model = model,
    };
    config_server_evt_act(CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_DELETE_ALL,&evt_param);
}

uint8_t config_model_subscription_virt_add(mesh_elmt_t *elmt,model_base_t *model,virt_addr_mngt_t* p_addr)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    for(i=0;i<model->subscription_list_size;++i)
    {
        if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) && model->subscription_list[i].addr.virt->virt_addr == p_addr->virt_addr)
        {
            i = model->subscription_list_size + 1;
            break;
        }
    }

    if(i <= model->subscription_list_size) {
        for(i=0;i<model->subscription_list_size;++i)
        {
            if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) == false)
            {
                model->subscription_list[i].addr.virt = p_addr;
                model->subscription_list[i].is_virt = true;
                node_save_subscription_list(model , i);
                LOG(3, "%s %d i = %d addr:%x\n", __func__, __LINE__, i, model->subscription_list[i].addr.virt->virt_addr);
                break;
            }
        }
    }

    if(i ==model->subscription_list_size + 1) {
        return Invalid_Address;
    }else if(i == model->subscription_list_size) {
        return Storage_Failure;
    }else {
        return Config_Success;
    }
    
}

uint8_t config_model_subscription_virt_delete(mesh_elmt_t *elmt,model_base_t *model,virt_addr_mngt_t* p_addr)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    for(i=0;i<model->subscription_list_size;++i)
    {
        if(model->subscription_list[i].is_virt == true &&  !memcmp(model->subscription_list[i].addr.virt->label_uuid, p_addr->label_uuid, LABEL_UUID_SIZE))
        {
            free_virt_addr_buf(model->subscription_list[i].addr.virt);
            model->subscription_list[i].addr.addr = 0;
            model->subscription_list[i].is_virt = false;
            model->subscription_list[i].addr.virt = NULL;
            node_delete_subscription_list(model , i);
            break;
        }
    }
    if(i == model->subscription_list_size) { 
        return Invalid_Address;
    }else {
        return Config_Success;
    }
}

uint8_t config_model_subscription_virt_overwrite(mesh_elmt_t *elmt,model_base_t *model,virt_addr_mngt_t* p_addr)
{
    if(model->subscription_list == NULL)
    {
        return Not_a_Subscribe_Model;
    }
    uint8_t i;
    for(i=0;i<model->subscription_list_size;++i)
    {
        
        if(!(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) == false) && (model->subscription_list[i].is_virt == true))
        {
            LOG(3, "config_model_subscription_virt_overwrite:%x %x\n", i, model->subscription_list[i].addr.virt->virt_addr);
            free_virt_addr_buf(model->subscription_list[i].addr.virt);

            model->subscription_list[i].addr.addr = 0;
            model->subscription_list[i].is_virt = false;
            model->subscription_list[i].addr.virt = NULL;
            node_delete_subscription_list(model , i);
            break;
        }
    }
    for(i=0;i<model->subscription_list_size;++i)
    {
        if(IS_MESH_ADDR_STRUCT_VALID(&model->subscription_list[i]) == false)
        {
            model->subscription_list[i].addr.virt = p_addr;
            model->subscription_list[i].is_virt = true;
            node_save_subscription_list(model , i);
            LOG(3, "2config_model_subscription_virt_overwrite:%x %x\n", i, model->subscription_list[i].addr.virt->virt_addr);
            break;
        }
    }

    if(i == model->subscription_list_size) { 
        return Invalid_Address;
    }else {
        return Config_Success;
    }
}

void config_model_subscription_virt_update_rx_cb(model_base_t *model, access_pdu_param_t *access_param, virt_addr_mngt_t *p_addr)
{
    uint8_t status = Config_Success;
    LOG(3, "config_model_subscription_virt_update_rx_cb virt_addr:%x\n", p_addr->virt_addr);
    do
    {
        mesh_elmt_t *dst_elmt = get_elmt_by_uni_addr(access_param->elmt_addr);
        if(dst_elmt == NULL)
        {
            status = Invalid_Address;
            break;
        }
        model_base_t *dst_model = get_model_inst_from_elmt(dst_elmt, access_param->model_id, access_param->sig_model);
        if(dst_model == NULL)
        {
            status = Invalid_Model;
            break;
        }
        switch(access_param->opcode & 0x3fff)
        {
            case Config_Model_Subscription_Virtual_Address_Add:
                status = config_model_subscription_virt_add(dst_elmt, dst_model, p_addr);
 //               type = CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD;
                //user callback
                if(status == Config_Success)
                {
                    config_server_evt_param_t evt_param;
                    evt_param.model_subscription_virt_addr_add.model = model;
                    memcpy(evt_param.model_subscription_virt_addr_add.label_uuid, p_addr->label_uuid, LABEL_UUID_SIZE);
                    config_server_evt_act(CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_ADD, &evt_param);
                }
                break;
            case Config_Model_Subscription_Virtual_Address_Delete:
                status = config_model_subscription_virt_delete(dst_elmt, dst_model, p_addr);
                //user callback
                if(status == Config_Success)
                {
                    config_server_evt_param_t evt_param;
                    evt_param.model_subscription_virt_addr_delete.model = model;
                    memcpy(evt_param.model_subscription_virt_addr_delete.label_uuid, p_addr->label_uuid, LABEL_UUID_SIZE);
                    config_server_evt_act(CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE,&evt_param);
                }
 //               type = CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_DELETE;
                break;
            case Config_Model_Subscription_Virtual_Address_Overwrite:
                status = config_model_subscription_virt_overwrite(dst_elmt, dst_model, p_addr);
 //               type = CONFIG_SERVER_EVT_MODEL_SUBSCRIPTION_VIRTUAL_ADDRESS_OVERWRITE;
                break;
            default:
                BX_ASSERT(0);
                break;
        }
    }while (0);
    uint8_t resp_buf[9] = {0};
    resp_buf[0] = status;
    memcpy(&resp_buf[1], &access_param->elmt_addr, 2);
    if(status == 0) {
        memcpy(&resp_buf[3], &p_addr->virt_addr, 2);
        memcpy(&resp_buf[5], &access_param->model_id, access_param->sig_model ? 2 : 4);
    }
    fd_model_two_octets_status_tx(access_param->netkey_global_idx, access_param->src_addr, &access_param->rx_time, config_model_subscription_status_tx_done,resp_buf,access_param->sig_model?7:9,Config_Model_Publication_Status);
}
extern void save_virt_addr_param(model_base_t *model, access_pdu_param_t *access_param, void (*cb)(model_base_t *, access_pdu_param_t *, virt_addr_mngt_t *));
void config_model_subscription_virt_update_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint8_t subscribe_addr[LABEL_UUID_SIZE];
    uint8_t * access = access_get_pdu_payload(pdu);
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    access_pdu_param_t access_param;
    uint16_t elmt_addr;
    bool sig_model;
    uint32_t model_id;

    memcpy(&elmt_addr, access + 2, 2);
    memcpy(subscribe_addr, access + 4, 16);
    if(get_access_pdu_rx_payload_size(pdu) == 24)
    {
        sig_model = false;
        memcpy(&model_id,access + 20,4);
    }else
    { 
        sig_model = true;
        model_id= access[20] | (access[21]<<8);
    }
    uint16_t opcode = (access[0] << 8) | access[1];
    LOG(3, "config_model_subscription_virt_update_rx opcode:%x model_id:%x\n", opcode, model_id);
    uint8_t status;
    access_param.elmt_addr = elmt_addr;
    access_param.sig_model = sig_model;
    access_param.model_id = model_id;
    access_param.opcode = opcode;
    access_param.netkey_global_idx = netkey_global_idx;
    access_param.src_addr = access_get_pdu_src_addr(pdu),
    access_param.rx_time =   access_rx_get_rx_time(pdu);

    save_virt_addr_param(model, &access_param, config_model_subscription_virt_update_rx_cb);
    virt_addr_add(subscribe_addr, &status);
}

static void config_vendor_model_subscription_list_tx_done(void *ptr,uint8_t status)
{
    LOG(3,"config_model_app_list_tx_done\n");
}

void config_SIG_model_subscription_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint16_t elmt_addr;
    uint32_t model_id;
    uint16_t netkey_global_idx = access_get_pdu_netkey_global_index(pdu);
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t status = Config_Success;
    model_base_t *dst_model = NULL;
    int addr_count = 0;

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

    uint8_t resp_buf[11];
    resp_buf[0] = status;
    memcpy(&resp_buf[1], access + 2, 4);
    LOG(3, "config_SIG_model_subscription_get_rx elmt_addr:%x model_id:%x status:%d dst_model:%x %p\n", elmt_addr, model_id, status, dst_model, dst_model);

    if(status == 0) {
        for(int i = 0; i < dst_model->subscription_list_size; i++) {
            LOG(3, "addr ===============i = %x %x %x\n", i, addr_count, dst_model->subscription_list[i].is_virt);
            if(dst_model->subscription_list[i].is_virt) {
                LOG(3, "virt ===============i = %x %x %x %x\n", i, addr_count, dst_model->subscription_list[i].addr.virt, dst_model->subscription_list[i].addr.virt->virt_addr);
                memcpy(&resp_buf[5 + addr_count++ * 2], &dst_model->subscription_list[i].addr.virt->virt_addr, 2);
            }else {
                if(dst_model->subscription_list[i].addr.addr) {
                    LOG(3, "addr ===============i = %x %x %x\n", i, addr_count, dst_model->subscription_list[i].addr.addr);
                    memcpy(&resp_buf[5 + addr_count++ * 2], &dst_model->subscription_list[i].addr.addr, 2);
                }
            }

        }
    }

    fd_model_two_octets_status_response_tx(pdu, config_vendor_model_subscription_list_tx_done,
        resp_buf, 5 + addr_count * 2, Config_SIG_Model_Subscription_List);
}

void config_vendor_model_subscription_get_rx(mesh_elmt_t *elmt, model_base_t *model, access_pdu_rx_t *pdu)
{
    uint16_t elmt_addr;
    uint32_t model_id;
    uint8_t *access = access_get_pdu_payload(pdu);
    uint8_t status = Config_Success;
    model_base_t *dst_model = NULL;
    int addr_count = 0;

    elmt_addr = *(uint16_t *)(access + 2);
    model_id = (*(uint32_t *)(access + 4));

    LOG(3, "config_vendor_model_subscription_get_rx elmt_addr:%x model_id:%x\n", elmt_addr, model_id);
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

    uint8_t resp_buf[13] = {0};
    resp_buf[0] = status;
    memcpy(&resp_buf[1], access + 2, 6);

    if(status == 0) {
        for(int i = 0; i < dst_model->subscription_list_size; i++) {
            if(IS_MESH_ADDR_STRUCT_VALID(&dst_model->subscription_list[i]) == false)
                continue;
#if 0
            if((dst_model->subscription_list[i].addr.addr & 0xf000) == 0xf000)
                continue;
#endif

            if(dst_model->subscription_list[i].is_virt)
                memcpy(&resp_buf[7 + addr_count++ * 2], &dst_model->subscription_list[i].addr.virt->virt_addr, 2);
            else
                memcpy(&resp_buf[7 + addr_count++ * 2], &dst_model->subscription_list[i].addr.addr, 2);
        }
    }

    fd_model_two_octets_status_response_tx(pdu, config_vendor_model_subscription_list_tx_done, 
        resp_buf, 7 + addr_count * 2, Config_Vendor_Model_Subscription_List);
}


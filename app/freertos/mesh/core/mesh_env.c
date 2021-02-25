#include "osapp_config.h"
#include <stdbool.h>
#include "sdk_mesh_definitions.h"
#include "bx_ring_queue.h"
#include "co_list.h"
#include "security.h"
#include "co_utils.h"
#include "osapp_config.h"
#include "virt_addr_mngt.h"
#include "beacon.h"
#include "mesh_env.h"
#include "osapp_mesh.h"
#include "node_save.h"
#include "sdk_mesh_config.h"
#include "config_server.h"
#include "device_keys_dm.h"
#include "app_keys_dm.h"
#include "device_manager_api.h"
#include "mesh_core_api.h"

void init_elmt_addr(uint16_t addr)
{
    uint8_t i;
    mesh_elmt_t *elmt = get_mesh_elmt();
    for(i = 0 ;i<get_element_num();++i)
    {
        elmt[i].uni_addr = addr++;
    }
}

void model_publish_subscribe_bind(model_base_t *model,model_publish_state_t *publish,mesh_addr_t *subscription_list,uint8_t subscription_list_size,model_publish_timeout_cb_t publish_period_cb)
{
    model->publish = publish;
    model->publish->publish_period_cb = publish_period_cb;
    model->subscription_list = subscription_list;
    model->subscription_list_size = subscription_list_size;

    for(int i = 0; i < subscription_list_size; i++)
        model->subscription_list[i].is_virt = false;
}

void mesh_model_init(model_base_t *model,uint32_t model_id,bool sig_model,uint8_t bound_key_buf_size,app_key_t **bound_key_buf)
{
    model->model_id = model_id;
    model->sig_model = sig_model;
    model->bound_key_buf_size = bound_key_buf_size;
    model->bound_key_buf = bound_key_buf;
    for(int i = 0; i < bound_key_buf_size; i++)
    {
        bound_key_buf[i] = NULL;
    }

}

mesh_elmt_t *get_elmt_by_uni_addr(uint16_t uni_addr)
{
    uint8_t i;
    mesh_elmt_t *elmt = get_mesh_elmt();
    for(i=0;i<get_element_num();++i)
    {
        if(elmt[i].uni_addr == uni_addr)
        {
            return &elmt[i];
        }
    }
    return NULL;
}


/**
 ****************************************************************************************
 * @brief The mesh node get primary element addr.
 *     (the unicast address of its primary element)
 *
 * @return uni_addr  primary element unicast addr.
 *
 ****************************************************************************************
 */
uint16_t mesh_node_get_primary_element_addr(void)
{
    mesh_elmt_t * p_elmt = get_mesh_elmt();
    BX_ASSERT(p_elmt);
    return p_elmt->uni_addr;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



dm_netkey_handle_t get_netkey_by_dst_addr(uint16_t addr)
{
    dm_netkey_handle_t netkey_handle;
    //TODO
    if(MESH_CORE_SUCCESS != dm_netkey_get_first_handle(addr,&netkey_handle))
    {
       return MESH_INVALID_NETKEY_HANDLE;
    }
    return netkey_handle;
}


model_base_t *get_model_inst_from_elmt(mesh_elmt_t *elmt,uint32_t model_id,bool sig_model)
{
    struct co_list_hdr *hdr = co_list_pick(&elmt->model_list);
    while(hdr)
    {
        model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
        //LOG(3,"get_model_inst_from_elmt:model_id:%x %xsig_model:%x %x\n", model_id, model->model_id, sig_model, model->sig_model);
        if(model->model_id == model_id && model->sig_model == sig_model)
        {
            return model;
        }
        hdr = co_list_next(hdr);
    }
    return NULL;
}

bool appkey_model_check(model_base_t *model_inst,app_key_t *appkey)
{
    uint8_t i;
    for(i=0;i<model_inst->bound_key_buf_size && model_inst->bound_key_buf[i]; ++i)
    {
        if(model_inst->bound_key_buf[i] == appkey)
        {
            return true;
        }
    }
    return false;
}

uint8_t *get_encryption_key(key_ptr_t *ptr)
{
    return ptr->key.net->key[ptr->idx].master.encryption_key;
}

uint8_t *get_privacy_key(key_ptr_t *ptr)
{
    return ptr->key.net->key[ptr->idx].master.privacy_key;
}

uint8_t ivi_nid_gen(uint32_t iv_index,key_ptr_t *netkey)
{
    return ((iv_index&0x1)<<7) |get_nid(netkey);
}

uint8_t get_nid(key_ptr_t *netkey)
{
    return netkey->key.net->key[netkey->idx].master.nid;
}

uint8_t get_ttl(mesh_elmt_t *elmt)
{
    //TODO
    return 0;
}

uint8_t *get_netkey(key_ptr_t *ptr)
{
    return ptr->key.net->key[ptr->idx].netkey;
}

uint8_t get_aid(key_ptr_t *ptr)
{
return ptr->key.app->key[ptr->idx].aid;
}

uint8_t *get_appkey(key_ptr_t *ptr)
{
    return ptr->key.app->key[ptr->idx].appkey;
}




dm_netkey_handle_t mesh_sec_beacon_get_netlist(dm_netkey_pos_t index)
{
    dm_netkey_handle_t netkey_handle;
   if(MESH_CORE_SUCCESS != dm_netkey_pos_to_handle(index,&netkey_handle))
   {
      return NULL;
   }
   return netkey_handle;
}

err_t mesh_sec_beacon_get_net_index(uint8_t *network_id,dm_netkey_pos_t *p_index)
{
    dm_netkey_pos_t keyIndex = 0;
    dm_netkey_handle_t netkey_handle ;

    if(MESH_CORE_SUCCESS != dm_netkey_network_id_to_handle(network_id,&netkey_handle))
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    if(MESH_CORE_SUCCESS !=dm_netkey_handle_to_pos(netkey_handle,&keyIndex))
    {
       return MESH_CORE_ERROR_INVALID_PARAM;
    }
    *p_index = keyIndex;
    return MESH_CORE_SUCCESS;


}

dm_netkey_handle_t mesh_sec_beacon_get_netlist_by_netid(uint8_t *network_id)
{
    dm_netkey_handle_t netkey_handle ;

    if(MESH_CORE_SUCCESS != dm_netkey_network_id_to_handle(network_id,&netkey_handle))
    {
        return NULL;
    }

    return netkey_handle;
}


uint8_t * get_devkey(uint16_t src_addr, uint16_t dst_addr)
{
    dev_key_t *devkey;
    mesh_core_params_t core_param;
    mesh_core_params_get(MESH_CORE_PARAM_MESH_ROLES , &core_param);
    if(core_param.role == MESH_ROLE_CONFIG_CLIENT)//client
    {
        dm_devkey_uniaddr_to_handle_get(dst_addr , &devkey);     //dst
    }
    else//server
    {
        dm_devkey_uniaddr_to_handle_get(src_addr , &devkey);     //src
    }
    
    return (uint8_t *)devkey->key;
}
uint8_t * get_peer_devkey(uint16_t src_addr, uint16_t dst_addr)
{
    return get_devkey(dst_addr,src_addr);
}

void  mesh_free(void * ptr)
{
    vPortFree(ptr);
}

void * mesh_alloc(size_t xWantedSize)
{
   return pvPortMalloc(xWantedSize);
}

bool is_lpn_node(void)
{
    mesh_core_params_t low_power;
    mesh_core_params_get(MESH_CORE_PARAM_FEATURE_LOW_POWER   , &low_power);

    if(MESH_FEATURE_ENABLED !=low_power.low_power)
    {
        return false;
    }else{
        return true;
    }

}

void set_is_provisioned(bool value)
{
    mesh_core_params_t param;
    param.is_provisioned = value;
    mesh_core_params_set(MESH_CORE_PARAM_IS_PROVISIONED , &param);
}

bool get_is_provisioned(void)
{
    mesh_core_params_t param;
    mesh_core_params_get(MESH_CORE_PARAM_IS_PROVISIONED , &param);
    return param.is_provisioned;
}


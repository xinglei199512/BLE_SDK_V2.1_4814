/*
 * node_save_model.c
 *
 *  Created on: 2018-8-22
 *      Author: jiachuang
 */

#include "osapp_config.h"
#include "node_save.h"
#include "node_save_macro.h"
#include "node_save_model.h"
#include "node_save_common.h"
#include "mesh_node_base.h"
#include "sdk_mesh_config_pro.h"
#include "device_keys_dm.h"
#include "network_keys_dm.h"
#include "app_keys_dm.h"
#include "bxfs.h"
#include "mesh_core_api.h"
#include "model_publish.h"

/*************** bind appkey*****************/


void node_save_bind_appkey(model_base_t *model , uint16_t appkey_idx , uint8_t save_keybuf_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_bind_appkey\n");

    //get dir and filename
    uint8_t file_name   = save_keybuf_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_model   = get_model_dir_index_in_its_element(model);
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_BOUNDKEY_LIST;

    //save to bxfs
    bxfs_write3(dir_element , dir_model , dir_key , file_name , &appkey_idx , MESH_SIZE_BOUND_KEY);
    node_save_write_through();
}


void node_save_unbind_appkey(model_base_t *model , uint8_t save_keybuf_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_unbind_appkey\n");

    //get dir and filename
    uint8_t file_name   = save_keybuf_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_model   = get_model_dir_index_in_its_element(model);
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_BOUNDKEY_LIST;
    //delete
    bxfs_delete3(dir_element , dir_model , dir_key , file_name);
    node_save_write_through();

}

void node_recover_bind_appkey(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_bind_appkey\n");

    uint8_t i=0,element_idx=0,model_idx=0;
    //uint8_t save_in_config_server_index;
    //app_key_t * appkey;
    uint16_t appkey_idx;

    uint8_t file_name   ;
    uint8_t dir_element = 0;
    uint8_t dir_model   = 0;
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_BOUNDKEY_LIST;

    for(element_idx=0;element_idx<get_element_num();element_idx++)
    {
        dir_element = MESHDIR1_ELMT_MIN + element_idx;
        model_idx = 0;
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[element_idx].model_list);
        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            model_idx ++;
            dir_model = model_idx;
            if((model->model_id != CONFIGURATION_SERVER_MODEL_ID) && (model->model_id != CONFIGURATION_CLIENT_MODEL_ID))//not config model
            {
                bxfs_listfile3(dir_element,dir_model,dir_key);
                for(i=0;i<get_node_save_file_count();i++)
                {
                    file_name = get_node_save_file_list(i);
                    bxfs_read3(dir_element,dir_model,dir_key,file_name,&appkey_idx,MESH_SIZE_BOUND_KEY);
                    dm_appkey_handle_t appkey_handle = MESH_INVALID_APPKEY_HANDLE;
                    dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle);
//                    appkey = app_key_find(get_config_server()->appkey , CONFIG_SERVER_APPKEY_LIST_MAX , appkey_idx);
                    model->bound_key_buf[i] = appkey_handle;
                }
            }
            hdr = co_list_next(hdr);
        }
    }
}

/*************** bind appkey*****************/





/***************subscription_list*****************/

void node_save_subscription_list(model_base_t *model , uint8_t save_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_subscription_list\n");

    //get dir and filename
    uint8_t file_name   = save_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_model   = get_model_dir_index_in_its_element(model);
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_SUBSCRIPTION_LIST;

    //save to bxfs
    bxfs_write3(dir_element , dir_model , dir_key , file_name , &model->subscription_list[save_index] , MESH_SIZE_SUBSCRIPTION_LIST);
    node_save_write_through();

}

/***************subscription_list*****************/

void node_delete_subscription_list(model_base_t *model , uint8_t save_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_subscription_list\n");

    //get dir and filename
    uint8_t file_name   = save_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_model   = get_model_dir_index_in_its_element(model);
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_SUBSCRIPTION_LIST; 
    bxfs_delete3(dir_element , dir_model , dir_key , file_name);
    node_save_write_through();

}


void node_recover_subscription_list(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_subscription_list\n");

    uint8_t i=0,element_idx=0,model_idx=0;
    mesh_addr_t *buf;

    uint8_t file_name   ;
    uint8_t dir_element = 0;
    uint8_t dir_model   = 0;
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_SUBSCRIPTION_LIST;

    for(element_idx=0;element_idx<get_element_num();element_idx++)
    {
        dir_element = MESHDIR1_ELMT_MIN + element_idx;
        model_idx = 0;
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[element_idx].model_list);
        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            model_idx ++;
            dir_model = model_idx;
            if((model->model_id != CONFIGURATION_SERVER_MODEL_ID) && (model->model_id != CONFIGURATION_CLIENT_MODEL_ID))//not config model
            {
                bxfs_listfile3(dir_element,dir_model,dir_key);
                for(i=0;i<get_node_save_file_count();i++)
                {
                    file_name = get_node_save_file_list(i);
                    buf = &model->subscription_list[i];
                    bxfs_read3(dir_element,dir_model,dir_key,file_name,buf,MESH_SIZE_SUBSCRIPTION_LIST);
                }
            }
            hdr = co_list_next(hdr);
        }
    }
}


/***************subscription_list*****************/




/***************publish_state*****************/

void node_save_publish_list(model_base_t *model , uint8_t save_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_save_publish_list\n");

    //get dir and filename
    uint8_t file_name   = save_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_model   = get_model_dir_index_in_its_element(model);
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_PUBLICATION;

    //save to bxfs
    bxfs_write3(dir_element , dir_model , dir_key , file_name , model->publish , MESH_SIZE_PUBLICATION);
    node_save_write_through();

}

void node_delete_publish_list(model_base_t *model , uint8_t save_index)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_delete_publish_list\n");

    //get dir and filename
    uint8_t file_name   = save_index;
    uint8_t dir_element = get_element_save_dir(model->elmt);
    uint8_t dir_model   = get_model_dir_index_in_its_element(model);
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_PUBLICATION;
 
    bxfs_delete3(dir_element , dir_model , dir_key , file_name);
    node_save_write_through();

}

void node_recover_publish_list(void)
{
    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node_recover_publish_list\n");

    uint8_t element_idx=0,model_idx=0;
    model_publish_state_t *buf;

    uint8_t file_name   ;
    uint8_t dir_element = 0;
    uint8_t dir_model   = 0;
    uint8_t dir_key     = MESHDIR3_ELMTX_MODELX_PUBLICATION;

    for(element_idx=0;element_idx<get_element_num();element_idx++)
    {
        dir_element = MESHDIR1_ELMT_MIN + element_idx;
        model_idx = 0;
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[element_idx].model_list);
        while(hdr)
        {
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            model_idx ++;
            dir_model = model_idx;
            if((model->model_id != CONFIGURATION_SERVER_MODEL_ID) && (model->model_id != CONFIGURATION_CLIENT_MODEL_ID))//not config model
            {
                bxfs_listfile3(dir_element,dir_model,dir_key);
                if(get_node_save_file_count() != 1)
                {
                    if(NODE_SAVE_LOG_ENABLE) LOG(LOG_LVL_INFO,"NODESAVE:node publish addr not found %d \n",get_node_save_file_count());
                }
                else
                {
                    file_name = get_node_save_file_list(0);
                    buf = model->publish;
                    bxfs_read3(dir_element,dir_model,dir_key,file_name,buf,MESH_SIZE_PUBLICATION);
                    model_publish_period_set(model->publish,model->publish->publish_period_cb,(void *)model);
                }
            }
            hdr = co_list_next(hdr);
        }
    }
}

/***************publish_state*****************/



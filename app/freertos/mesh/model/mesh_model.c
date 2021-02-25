/**
 ****************************************************************************************
 *
 * @file   mesh_model.c
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2018-10-12 18:43
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "mesh_model.h"
#include "mesh_core_api.h"
#include "config_server.h"
#include "device_keys_dm.h"
#include "app_keys_dm.h"
#include "device_manager_api.h"
#include "mesh_core_api.h"
#include "node_save.h"
#include "foundation_common.h"
#include "config_server_feature.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   Func   bind_appkey_to_model_by_elem
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

app_key_t **alloc_bound_appkey_buf(app_key_t **buf,uint8_t size,uint8_t *save_keybuf_index)
{
    uint8_t i;
    for(i =0 ;i<size;++i)
    {
        if(buf[i]== NULL)
        {
            *save_keybuf_index = i;
            return &buf[i];
        }
    }
    return NULL;
}

bool appkey_is_bound(app_key_t **buf,uint8_t size,app_key_t *appkey)
{
    uint8_t i;
    for(i =0 ;i<size;++i)
    {
        if(buf[i]== appkey)
        {
            return true;
        }
    }
    return false;
}

err_t bind_appkey_to_model_by_element(uint8_t elem_idx,uint32_t model_id,uint16_t appkey_idx)
{
    err_t err= MESH_CORE_SUCCESS;

    if(elem_idx<ELEMENT_NUM)
    {
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[elem_idx].model_list);
        while(hdr)
        {
            uint8_t status;
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            if(model->model_id == model_id)
            {
                bind_appkey_to_model(model,appkey_idx,&status);
                err = status;
                break;
            }
            hdr = co_list_next(hdr);
        }
    }
    else
    {
        err=  err= MESH_CORE_ERROR_INVALID_PARAM;;
    }
    return err;
}
/**
 ****************************************************************************************
 * @brief   Func    bind_appkey_to_model
 *
 ****************************************************************************************
 */
void bind_appkey_to_model(model_base_t *model,uint16_t appkey_idx,uint8_t *status)
{
    uint8_t save_keybuf_index = 0;
    dm_appkey_handle_t appkey_handle;
    err_t err;

    err = dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle);
    if(err == MESH_CORE_SUCCESS)
    {
        if(appkey_is_bound(model->bound_key_buf,model->bound_key_buf_size, appkey_handle))
        {
            *status = Config_Success;
            LOG(3,"bind_appkey_is_bound\n");
        }
        else
        {
            app_key_t **bound_buf = alloc_bound_appkey_buf(model->bound_key_buf,model->bound_key_buf_size,&save_keybuf_index);
            if(bound_buf)
            {
                *bound_buf = appkey_handle;
                node_save_bind_appkey(model , appkey_idx , save_keybuf_index);
                *status = Config_Success;
                LOG(3,"bind_appkey_to_model-Config_Success\n");
            }else
            {
                *status = Insufficient_Resources;
                LOG(3,"bind_appkey_to_model-Insufficient_Resources\n");
            }
        }
    }else
    {
        *status = Invalid_AppKey_Index;
        LOG(3,"bind_appkey_to_model-Invalid_AppKey_Index\n");
    }
}

void free_bound_appkey_buf(app_key_t **buf,uint8_t size,app_key_t *appkey,uint8_t *save_keybuf_index)
{
    uint8_t i;
    for(i =0 ;i<size;++i)
    {
        if(buf[i]== appkey)
        {
            *save_keybuf_index = i;
            buf[i] = NULL;
            return;
        }
    }
}

/**
 ****************************************************************************************
 * @brief   Func    unbind_appkey_to_model
 *
 ****************************************************************************************
 */
void unbind_appkey_to_model(model_base_t *model,uint16_t appkey_idx,uint8_t *status)
{
    uint8_t save_keybuf_index = 0;
    dm_appkey_handle_t appkey_handle;
    err_t err;

    err = dm_appkey_index_to_appkey_handle(appkey_idx,&appkey_handle);
    if(err == MESH_CORE_SUCCESS)
    {
        if(appkey_is_bound(model->bound_key_buf,model->bound_key_buf_size, appkey_handle))
        {
             save_keybuf_index = 0xff;
             free_bound_appkey_buf(model->bound_key_buf,model->bound_key_buf_size, appkey_handle,&save_keybuf_index);
             if(save_keybuf_index != 0xff )
             {
                 node_save_unbind_appkey(model , save_keybuf_index);
             }
        }
        *status = Config_Success;
        LOG(3,"bind_appkey_is_unbound\n");
    }else
    {
        *status = Invalid_AppKey_Index;
        LOG(3,"bind_appkey_to_model-Invalid_AppKey_Index\n");
    }
}
/**
 ****************************************************************************************
 * @brief   Func    unbind_appkey_to_all_model
 *
 ****************************************************************************************
 */
void unbind_appkey_to_all_model(dm_appkey_handle_t p_appkey_handle)
{
    uint8_t i=0;
    for(i=0;i<ELEMENT_NUM;i++)
    {
        struct co_list_hdr *hdr = co_list_pick(&get_mesh_elmt()[i].model_list);
        while(hdr)
        {
            uint8_t j=0;
            model_base_t *model = CONTAINER_OF(hdr, model_base_t, hdr);
            for(j=0;j<model->bound_key_buf_size;j++)
            {
                if(model->bound_key_buf[j] == p_appkey_handle)
                {
                    model->bound_key_buf[j] = NULL;
                    //save to  flash
                    node_save_unbind_appkey(model , j);
                }
            }
            hdr = co_list_next(hdr);
        }
    }
}
/**
 ****************************************************************************************
 * @brief   Func    get_first_appkey_global_idx_in_model
 * @param[in] model the model base
 * @return          the global index
 ****************************************************************************************
 */
uint16_t get_first_appkey_global_idx_in_model(model_base_t *model)
{
    uint8_t i=0;
    for(i=0;i<APPKEY_BOUND_NETKEY_MAX_NUM;i++)
    {
        if(model->bound_key_buf[i] != 0)
        {
            return model->bound_key_buf[i]->global_idx;
        }
    }
    return 0xFFFF;//not found
}




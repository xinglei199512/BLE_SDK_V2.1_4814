/**
 ****************************************************************************************
 *
 * @file   app_keys_dm.c
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2018-09-15 10:22
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

#include "app_keys_dm.h"
#include "network_keys_dm.h"
#include "static_buffer.h"
//#include "network.h"
#include "node_save_keys.h"
#include "security.h"

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
typedef enum
{
   GENERATE_KEYBOX_PHASE_PRIMARY = 0,
   GENERATE_KEYBOX_PHASE_UPDATE = 1,
   GENERATE_KEYBOX_PHASE_USER_CB = 2,
}appkey_generate_keybox_phase_t;


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef struct
{
   dm_async_cb_t func_done;
   dm_appkey_handle_t key_handle;
   appkey_generate_keybox_phase_t phase;//is ready to generate which auto.
}appkey_generate_keybox_t;

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
DEF_ARRAY_BUF(m_appkey_hdl,app_key_t,DM_CFG_APPKEY_MAX_NUM);

static volatile appkey_generate_keybox_t m_generate_keybox=
{
    .func_done = NULL,
    .key_handle = NULL,
    .phase = GENERATE_KEYBOX_PHASE_PRIMARY,
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void dm_appkey_set(dm_netkey_handle_t p_netkey_handle,mesh_global_idx_t appkey_idx, const uint8_t * p_key, dm_appkey_handle_t p_appkey_handle,dm_async_cb_t func_done);
static void dm_appkey_generate_aid_keys_callback(void);

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

uint8_t dm_appkey_get_update_index(dm_appkey_handle_t p_appkey_handle)
{
    return ( 1-p_appkey_handle->primary_used);
}

 
/**
 ****************************************************************************************
 * @brief   Func to search appkey which match aid and netkey handle,and do the action when it matchs.
 *
 * @param[in] aid                   The application key aid of the application key.
 * @param[in] p_netkey_handle       Pointer to the handle for the network key.
 * @param[in] action The callback   Function when search the match aid in application key pool.
 *
 * @return The result status of aid search.true when one/more key match ,false when no match key.
 ****************************************************************************************
 */
bool dm_appkey_aid_search(uint8_t aid,dm_netkey_handle_t p_netkey_handle,dm_appkey_aid_search_action action)
{
    /* aid match*/
    bool state = false;
    dm_appkey_handle_t l_handle;

    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
        if(l_handle->bound_netkey == p_netkey_handle)//bound netkey match
        {//check aid
           if(l_handle->bound_netkey->key_refresh_phase != MESH_KEY_REFRESH_PHASE_0)
           {
                //1. old key
                if ( l_handle->key[l_handle->primary_used].aid == aid)
                {
                    state = true;
                    action( l_handle,&(l_handle->key[l_handle->primary_used]) );
                }
                //2. new key
                if ( l_handle->key[dm_appkey_get_update_index(l_handle)].aid == aid)
                {
                    state = true;
                    action( l_handle,&(l_handle->key[dm_appkey_get_update_index(l_handle)]) );
                }
           }
           else
           {
                //only old key used
                if ( l_handle->key[l_handle->primary_used].aid == aid)
                {
                    state = true;
                    action( l_handle,&(l_handle->key[l_handle->primary_used]) );
                }
           }
        }
    );

    return state;
}

err_t dm_appkey_match(mesh_global_idx_t appkey_idx,uint8_t *key)
{
    dm_appkey_handle_t l_handle;
    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
        if(l_handle->global_idx == appkey_idx && !memcmp(l_handle->key[l_handle->primary_used].appkey, key, MESH_KEY_LENGTH))
        {
            return MESH_CORE_SUCCESS;
        }
    );
    return MESH_CORE_ERROR_NOT_FOUND;
}




/**
 ****************************************************************************************
 * @brief   Func to transform a application key index to a application key handle in the device manager.
 *
 * @param[in] appkey_idx   The application key index of the application key.
 * @param[out] pp_appkey_handle Pointer to the Pointer to the handle for the application key.
 *
 * @return The result status of the appkey appkey_index_to_appkey_handle cmd.
 ****************************************************************************************
 */
err_t dm_appkey_index_to_appkey_handle(mesh_global_idx_t appkey_idx,dm_appkey_handle_t * pp_appkey_handle)
{
    dm_appkey_handle_t l_handle;
    err_t err = MESH_CORE_ERROR_NOT_FOUND;
    
    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
       if(l_handle->global_idx == appkey_idx)
       {
           *pp_appkey_handle = l_handle;
           err = MESH_CORE_SUCCESS;
           break;
       }
    );

    return err;
}

/**
 ****************************************************************************************
 * @brief   Func to transform a application key handle to a application key index in the device manager.
 *
 * @param[in] p_appkey_handle   Pointer to the handle for the application key.
 * @param[out] p_appkey_idx     Pointer to the application key index of the application key.
 *
 * @return The result status of the appkey appkey_handle_to_appkey_index cmd.
 ****************************************************************************************
 */
err_t dm_appkey_handle_to_appkey_index(dm_appkey_handle_t p_appkey_handle, mesh_global_idx_t * p_appkey_idx)
{
    if(MESH_INVALID_APPKEY_HANDLE == p_appkey_handle || NULL == p_appkey_idx)
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    *p_appkey_idx = p_appkey_handle->global_idx;
    return MESH_CORE_SUCCESS;
    
}
/**
 ****************************************************************************************
 * @brief   Function to generate app key aid callback.
 *
 ****************************************************************************************
 */
static void dm_appkey_generate_aid_keys_callback(void)
{
    uint8_t array_index;
    //start generate
    switch(m_generate_keybox.phase)
    {
//        case GENERATE_KEYBOX_PHASE_UPDATE :
//            m_generate_keybox.phase = GENERATE_KEYBOX_PHASE_USER_CB;// primary is callback by system,and is to generate update auto.
//            k4_derivation(m_generate_keybox.key_handle->key[KEY_UPDATED].appkey, &(m_generate_keybox.key_handle->key[KEY_UPDATED].aid), dm_appkey_generate_aid_keys_callback);
//            break;
        case GENERATE_KEYBOX_PHASE_USER_CB:
            //1. flash save data
            array_index = array_buf_idx_get(&m_appkey_hdl , m_generate_keybox.key_handle);
            node_save_app_key_add(m_generate_keybox.key_handle , array_index);

            //2. user cb
            if(NULL != m_generate_keybox.func_done)
            {
                m_generate_keybox.func_done(m_generate_keybox.key_handle);//generate done,and just nofity the user.
            }
            break;
        default:
            LOG(LOG_LVL_ERROR, "!!! ERROR dm_appkey_generate_aid_keys_callback phase !!!\n");
            break;
    }
}

/**
 ****************************************************************************************
 * @brief   Func to save a app key and its associated app key index to the device manager.
 *
 * @param[in] p_netkey_handle   Pointer to the handle for the netkey.
 * @param[in] appkey_idx        The appkey index  being added.
 * @param[in] p_key             Pointer to the appkey, it must be @ref MESH_KEY_LENGTH bytes long.
 * @param[in] p_appkey_handle   Pointer to the handle for the appkey.
 * @param[in] func_done         The callback function when add appkey is done.
 *
 ****************************************************************************************
 */
static void dm_appkey_set(dm_netkey_handle_t p_netkey_handle,mesh_global_idx_t appkey_idx, const uint8_t * p_key, dm_appkey_handle_t p_appkey_handle,dm_async_cb_t func_done)
{
    p_appkey_handle->global_idx = appkey_idx;
    p_appkey_handle->bound_netkey = p_netkey_handle;
    memcpy(p_appkey_handle->key[p_appkey_handle->primary_used].appkey,p_key,MESH_KEY_LENGTH);

    //1. save value
    m_generate_keybox.func_done = func_done;
    m_generate_keybox.key_handle = p_appkey_handle;
    m_generate_keybox.phase = GENERATE_KEYBOX_PHASE_USER_CB;
    //2. generate aid keys
    k4_derivation(p_appkey_handle->key[p_appkey_handle->primary_used].appkey, &(p_appkey_handle->key[p_appkey_handle->primary_used].aid), dm_appkey_generate_aid_keys_callback);

}



/**
 ****************************************************************************************
 * @brief   Func to adds a application key and its associated application key index to the device manager.
 *
 * @param[in] netkey_idx        The network key index of the netkey being to bind appkey.
 * @param[in] appkey_idx        The application key index of the application key being added.
 * @param[in] p_key             Pointer to the application key, it must be @ref MESH_KEY_LENGTH bytes long.
 * @param[out] pp_appkey_handle Pointer to the Pointer to the handle for the application key.
 * @param[in] func_done         The callback function when add appkey is done.
 *
 * @return The result status of the appkey add cmd.
 ****************************************************************************************
 */
err_t dm_appkey_add(mesh_global_idx_t netkey_idx, mesh_global_idx_t appkey_idx, const uint8_t * p_key, dm_appkey_handle_t * pp_appkey_handle,dm_async_cb_t func_done)
{
    err_t err =  MESH_CORE_SUCCESS;

    dm_appkey_handle_t l_handle;
    dm_netkey_handle_t netkey_handle;

    if( (NULL == p_key) || (NULL == pp_appkey_handle))
    {
        err = MESH_CORE_ERROR_NULL;
    }
    else if(MESH_CORE_ERROR_NOT_FOUND == dm_netkey_index_to_netkey_handle(netkey_idx,&netkey_handle))//have in buf
    {
        err = MESH_DM_ERROR_INVALID_NETKEY_INDEX;
    }
    else if(MESH_CORE_SUCCESS == dm_appkey_index_to_appkey_handle(appkey_idx,&l_handle))//have in buf
    {
        err = MESH_DM_ERROR_KEY_INDEX_ALREADY_STORED;
    }
    else
    {
        l_handle = array_buf_calloc(&m_appkey_hdl);
        if(NULL == l_handle)
        {
            err = MESH_DM_ERROR_INSUFFICIENT_RESOURCES;
        }
        else
        {
            //1. local save
            dm_appkey_set(netkey_handle,appkey_idx,p_key,l_handle,func_done);

            //2. set dev handle
            *pp_appkey_handle = l_handle;
            //3. notify gatt  by send secret beacon
        }
    }

    return err;

}

/**
 ****************************************************************************************
 * @brief   Function To updates an existing application key.
 *
 *  This starts the key refresh procedure for this application, and causes the application
 *  to enter phase 1 of the procedure. In phase 1, the old keys are still used to transmit
 *  messages, but messages can be received using either the old or the new keys.
 *
 * @param[in] p_appkey_handle   Pointer to the handle for the application key.
 * @param[in] p_key             Pointer to the new application key to use, it must be @ref MESH_KEY_LENGTH bytes long.
 * @param[in] func_done         The callback function when update appkey is done.
 *
 * @return  The result status of the appkey update cmd.
 ****************************************************************************************
 */
err_t dm_appkey_update(dm_appkey_handle_t p_appkey_handle, const uint8_t * p_key,dm_async_cb_t func_done)
{
    err_t err =  MESH_CORE_SUCCESS;


    if( (NULL == p_key) || (MESH_INVALID_APPKEY_HANDLE == p_appkey_handle))
    {
        err = MESH_CORE_ERROR_NULL;
    }
    else if(!array_buf_is_elem_handle_valid(&m_appkey_hdl,p_appkey_handle))
    {
        err = MESH_CORE_ERROR_NOT_FOUND;
    }
    else
    {
        //1. local save
        memcpy(p_appkey_handle->key[dm_appkey_get_update_index(p_appkey_handle)].appkey,p_key,MESH_KEY_LENGTH);

        //2. generate aid
            //2.1. save value
        m_generate_keybox.func_done = func_done;
        m_generate_keybox.key_handle = p_appkey_handle;
        m_generate_keybox.phase = GENERATE_KEYBOX_PHASE_USER_CB;
            //2.2. generate aid keys
        k4_derivation(p_appkey_handle->key[dm_appkey_get_update_index(p_appkey_handle)].appkey, &(p_appkey_handle->key[dm_appkey_get_update_index(p_appkey_handle)].aid), dm_appkey_generate_aid_keys_callback);
    }

    return err;
}



/**
 ****************************************************************************************
 * @brief   Function To delete an existing application key from the device manager pool.
 *  All applications bound to the specified application will also be deleted.
 *
 * @param[in] p_appkey_handle   Pointer to the handle for the application key.
 *
 * @return  The result status of the appkey delete cmd.
 ****************************************************************************************
 */
err_t dm_appkey_delete(dm_appkey_handle_t p_appkey_handle)
{
    err_t err =  MESH_CORE_SUCCESS;

    if(NULL == p_appkey_handle)
    {
        err = MESH_CORE_ERROR_NULL;
    }
    else if(!array_buf_is_elem_handle_valid(&m_appkey_hdl,p_appkey_handle))
    {
        err = MESH_CORE_ERROR_NOT_FOUND;
    }
    else
    {
        /* Ensure that the subnetwork has no apps associated */
        //1. delete bound app model
        unbind_appkey_to_all_model(p_appkey_handle);
        //2. delete app handle in buf
        array_buf_release(&m_appkey_hdl,p_appkey_handle);
        //3. delete data in flash
        uint8_t array_index = array_buf_idx_get(&m_appkey_hdl , p_appkey_handle);
        node_save_app_key_delete(array_index);
    }

    return err;
}

bool dm_get_all_appkey_index(uint8_t *appkey_idx, uint32_t * p_count)
{

    bool state = false;
    dm_appkey_handle_t l_handle;
    uint32_t i = 0;

    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
           if(i % 2 == 0) {
                appkey_idx[i++] = (l_handle->global_idx & 0x0ff);
                appkey_idx[i] = ((l_handle->global_idx & 0xf00) >> 8);
            }else {
                appkey_idx[i++] |= ((l_handle->global_idx & 0x00f) << 4) ;
                appkey_idx[i] = (l_handle->global_idx & 0xff0) >> 4;
            }
           LOG(3, "dm_get_all_netkey_idx i=%x %x %x %x\n", i, appkey_idx[i-1], appkey_idx[i], l_handle->global_idx);
    );

    if(i) state = true;
    if(i % 2 == 1) {
        if(i == 1)
            *p_count = 2;
        else
            *p_count = i;
    }
    else
        *p_count = i + 1;

    return state;
}

/**
 ****************************************************************************************
 * @brief   Function To gets all the app key from the device manager pool.
 *
 * @param[out] pp_appkey_handle_list Pointer to the array for storing all the app key handle.
 * @param[out] p_count The size of the @c pp_app_handle_list array. Will be changed to the number of
 *
 * @return  false if the given list is not large enough to store all the available app key indices.
 ****************************************************************************************
 */
 bool dm_get_all_appkey_handles(dm_appkey_handle_t * pp_appkey_handle_list, uint32_t * p_count)
{

    bool state = false;
    dm_appkey_handle_t l_handle;
    uint32_t i=0;

    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
           pp_appkey_handle_list[i++] = l_handle;
    );

    if(i) state = true;
    *p_count = i;

    return state;
}


/**
 ****************************************************************************************
 * @brief   Function To gets all the application key from the device manager pool.
 *
 * @param[out] pp_appkey_handle_list Pointer to the array for storing all the application key handle.
 * @param[out] p_count The size of the @c pp_appkey_handle_list array. Will be changed to the number of
 *
 * @return  The result status of the appkey list get cmd.
 ****************************************************************************************
 */
err_t dm_appkey_handle_list_get(dm_appkey_handle_t * pp_appkey_handle_list, uint32_t * p_count)
{
     err_t err =  MESH_CORE_SUCCESS;
     
     if (NULL == pp_appkey_handle_list || NULL == p_count )
     {
         err =    MESH_CORE_ERROR_NULL;
     }
     else
     {
         dm_get_all_appkey_handles(pp_appkey_handle_list, p_count);
     }
     
     return err;

}



err_t dm_appkey_get_update_info(dm_appkey_handle_t p_appkey_handle, app_key_box_t ** pp_update_appkey_info)
{
    err_t err =  MESH_CORE_SUCCESS;
    if(MESH_INVALID_APPKEY_HANDLE == p_appkey_handle || NULL == pp_update_appkey_info)
    {
        return MESH_CORE_ERROR_NOT_FOUND;
    }
    *pp_update_appkey_info =&(p_appkey_handle->key[dm_appkey_get_update_index(p_appkey_handle)]);
    return err;
}


err_t dm_appkey_get_info(dm_appkey_handle_t p_appkey_handle, app_key_box_t ** pp_update_appkey_info)
{
    err_t err =  MESH_CORE_SUCCESS;
    if(MESH_INVALID_APPKEY_HANDLE == p_appkey_handle || NULL == pp_update_appkey_info)
    {
        return MESH_CORE_ERROR_NOT_FOUND;
    }
    *pp_update_appkey_info =&(p_appkey_handle->key[p_appkey_handle->primary_used]);
    return err;
}



/**
 ****************************************************************************************
 * @brief   Function To recovery appkey from bxfs.
 *
 * @param[in] idx               The index of the appkey in bxfs to set in the network key pool in array index position.
 * @param[in] p_handle          Pointer to the appkey node data recovery from bxfs.
 *
 * @return  The result status of the devkey recovery_from_bxfs cmd.
 ****************************************************************************************
 */
err_t dm_appkey_recovery_from_bxfs(uint16_t idx,app_key_t *p_handle)
{
    err_t err =  MESH_CORE_SUCCESS;

    if( NULL==p_handle )
    {
        err =  MESH_CORE_ERROR_NULL;
    }
    else if( idx >=DM_CFG_APPKEY_MAX_NUM)
    {
        err =  MESH_CORE_ERROR_INVALID_PARAM;
    }
    else
    {
        dm_appkey_handle_t l_handle = array_buf_calloc_idx(&m_appkey_hdl,idx);

        if(NULL == l_handle)
        {
            err = MESH_CORE_ERROR_NO_MEM;
        }
        else
        {
            // Add to dm system  memcpy
            *l_handle = *p_handle;
        }
    }
    return err;
}


/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

void dm_appkey_netkey_unbind_all(dm_netkey_handle_t p_netkey_handle)
{
    dm_appkey_handle_t l_handle;

    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle, 
        if(l_handle->bound_netkey == p_netkey_handle) 
        { 
            dm_appkey_delete(l_handle);//delete
        } 
    );
}

/**
 ****************************************************************************************
 * @brief   Function To judge appkey update info .
 *
 * @param[in] p_appkey_handle          Pointer to the appkey node .
 *
 * @return  The result status of the appkey update is valid.
 ****************************************************************************************
 */
static bool dm_appkey_is_valid_update(dm_appkey_handle_t        p_appkey_handle)
{
    uint8_t update_index = dm_appkey_get_update_index(p_appkey_handle);
    uint8_t invaild_key[MESH_KEY_LENGTH];
    memset(invaild_key,0,MESH_KEY_LENGTH);
    if(memcmp(p_appkey_handle->key[update_index].appkey, invaild_key, MESH_KEY_LENGTH))
    {
        return true;
    }else
    {
        return false;
    }
    
}


void dm_appkey_refresh_swap_keys(dm_netkey_handle_t p_netkey_handle)
{
    dm_appkey_handle_t l_handle;

    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle, 
        if(l_handle->bound_netkey == p_netkey_handle) 
        { 
           if(dm_appkey_is_valid_update(l_handle))
           {
               memset(l_handle->key[ l_handle->primary_used].appkey,0,MESH_KEY_LENGTH);
               l_handle->primary_used = dm_appkey_get_update_index(l_handle);
           }
        } 
    );
         
}

err_t dm_appkey_get_bound_netkey(dm_appkey_handle_t        p_appkey_handle, dm_netkey_handle_t* pp_netkey_handle)
{
    if(NULL == pp_netkey_handle || MESH_INVALID_APPKEY_HANDLE == p_appkey_handle)
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    *pp_netkey_handle = p_appkey_handle->bound_netkey;
    return MESH_CORE_SUCCESS;

}

void dm_set_appkey_ptr(dm_appkey_handle_t p_appkey_handle,key_ptr_t *key_ptr,bool use_new)
{
        key_ptr->key.app = p_appkey_handle;
        key_ptr->idx = use_new ? p_appkey_handle->primary_used:(dm_appkey_get_update_index(p_appkey_handle));
}

void dm_aid_search(uint8_t aid,key_ptr_t *appkey,uint8_t *matched_num)
{
    dm_appkey_handle_t l_handle;
    dm_netkey_handle_t netkey_handle;

    uint8_t l_matched_num = 0;


    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
        {

            if(MESH_CORE_SUCCESS !=dm_appkey_get_bound_netkey(l_handle,&netkey_handle))
            {
                continue;
            }
            if(l_handle->key[l_handle->primary_used].aid == aid && l_handle->key[dm_appkey_get_update_index(l_handle)].aid == aid && netkey_handle->key_refresh_phase !=  MESH_KEY_REFRESH_PHASE_0)
            {
                appkey[l_matched_num].key.app  = l_handle;
                appkey[l_matched_num].idx = l_handle->primary_used;
                l_matched_num += 1;
                appkey[l_matched_num].key.app  = l_handle;
                appkey[l_matched_num].idx = dm_appkey_get_update_index(l_handle);
                l_matched_num += 1;
                LOG(3,"dm_aid_search both !\n");
            }
            else if( l_handle->key[dm_appkey_get_update_index(l_handle)].aid == aid && netkey_handle->key_refresh_phase !=  MESH_KEY_REFRESH_PHASE_0)
            {
                appkey[l_matched_num].key.app  = l_handle;
                appkey[l_matched_num].idx = dm_appkey_get_update_index(l_handle);
                l_matched_num += 1;
                LOG(3,"dm_aid_search update !\n");
            }
            else if(l_handle->key[l_handle->primary_used].aid == aid)
            {
                appkey[l_matched_num].key.app  = l_handle;
                appkey[l_matched_num].idx = l_handle->primary_used;
                l_matched_num += 1;
                LOG(3,"dm_aid_search current! \n");
            }
        }
    );

    *matched_num = l_matched_num;
}


//err_t dm_appkey_pos_to_handle(dm_appkey_pos_t pos, dm_appkey_handle_t *pp_appkey_handle)
//{
//    err_t err =  MESH_CORE_ERROR_NOT_FOUND;
//    dm_appkey_handle_t l_handle;
//    dm_appkey_pos_t temp = 0;
//
//    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
//    {
//
//        if(pos == temp)
//        {
//            *pp_appkey_handle = l_handle;
//            return MESH_CORE_SUCCESS;
//        }
//        temp++;
//    }
//    );
//    return err;
//
//}

err_t dm_appkey_get_keybox_info(dm_appkey_handle_t        p_appkey_handle, app_key_box_t ** p_app_key_box)
{
    err_t err = MESH_CORE_SUCCESS;
    mesh_key_refresh_phase_t phase = MESH_KEY_REFRESH_PHASE_INVALID;
    dm_netkey_handle_t net_handle = MESH_INVALID_NETKEY_HANDLE;
    err = dm_appkey_get_bound_netkey(p_appkey_handle,&net_handle);
    if(err != MESH_CORE_SUCCESS)
    {
        return err;
    }
    err = dm_netkey_kr_phase_get(net_handle,&phase);
    if(err != MESH_CORE_SUCCESS)
    {
        return err;
    }
    if(phase == MESH_KEY_REFRESH_PHASE_2)
    {
        return dm_appkey_get_update_info(p_appkey_handle,p_app_key_box);
    }
    return dm_appkey_get_info(p_appkey_handle,p_app_key_box);
    
}

err_t dm_appkey_get_bound_netkey_box(dm_appkey_handle_t          p_appkey_handle, app_key_box_t * p_app_key_box,dm_netkey_handle_t * pp_netkey_handle,net_key_box_t ** pp_net_key_box)
{
    err_t err = MESH_CORE_SUCCESS;
    if(NULL == pp_netkey_handle || NULL == pp_net_key_box)
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    err = dm_appkey_get_bound_netkey(p_appkey_handle,pp_netkey_handle);
    if(err != MESH_CORE_SUCCESS)
    {
        return err;
    }
    if(NULL == *pp_netkey_handle)
    {
        return MESH_CORE_ERROR_INVALID_PARAM;
    }
    
    if(&(p_appkey_handle->key[p_appkey_handle->primary_used]) == p_app_key_box)
    {
        *pp_net_key_box = &(*pp_netkey_handle)->key[p_appkey_handle->primary_used];
    }
    else if(&(p_appkey_handle->key[dm_appkey_get_update_index(p_appkey_handle)]) == p_app_key_box)
    {
        *pp_net_key_box = &(*pp_netkey_handle)->key[dm_appkey_get_update_index(p_appkey_handle)];
    }
    else
    {
        return MESH_CORE_ERROR_NOT_FOUND;
    }
    return MESH_CORE_SUCCESS;
}



void aid_search(uint8_t aid,net_key_t *netkey,void (*candidate_add)(app_key_t *,app_key_box_t *))
{
    dm_appkey_handle_t l_handle;
    dm_netkey_handle_t netkey_handle;

    FOR_EACH_ALLOCATED_ITEM(m_appkey_hdl,l_handle,
        {
            if(MESH_CORE_SUCCESS !=dm_appkey_get_bound_netkey(l_handle,&netkey_handle))
            {
                continue;
            }
            if(l_handle->key[l_handle->primary_used].aid == aid && l_handle->key[dm_appkey_get_update_index(l_handle)].aid == aid && netkey_handle->key_refresh_phase !=  MESH_KEY_REFRESH_PHASE_0 
               && netkey_handle == netkey)
            {
                candidate_add(l_handle, &l_handle->key[l_handle->primary_used]);
                candidate_add(l_handle, &l_handle->key[dm_appkey_get_update_index(l_handle)]);
                LOG(3,"dm_aid_search both !\n");
            }
            else if( l_handle->key[dm_appkey_get_update_index(l_handle)].aid == aid && netkey_handle->key_refresh_phase !=  MESH_KEY_REFRESH_PHASE_0)
            {
                candidate_add(l_handle, &l_handle->key[dm_appkey_get_update_index(l_handle)]);
                LOG(3,"dm_aid_search update !\n");
            }
            else if(l_handle->key[l_handle->primary_used].aid == aid)
            {
                candidate_add(l_handle, &l_handle->key[l_handle->primary_used]);
                LOG(3,"dm_aid_search current! \n");
            }
        }
    );
}




/**
 ****************************************************************************************
 *
 * @file   bond_save.c
 *
 * @brief  ble bond save apis.
 *
 * @author  Chen Jia Chuang
 * @date    2018-12-24
 * @version <0.0.0.1>
 *
 * @license
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */
#include "bond_save.h"
#include "bond_save_flash.h"
#include "bond_save_db_cccd.h"
#include "bond_save_security.h"
#include "nvds_in_ram.h"
#include "string.h"
#include "log.h"
#include "osapp_config.h"
#include "osapp_task.h"
#include "plf.h"


/*
 * REMOVE OLDEST KEY , FIND NEW DEVICE TAG
 ****************************************************************************************
 */
static void bond_save_age_insert_new_item(uint8_t index , uint8_t *buff)
{
    uint8_t i=0;
    for(i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        if(i[buff] > 0)
        {
            i[buff] ++;
        }
    }
    index[buff] = 1;
}


static uint8_t get_oldest_item(uint8_t *buff)
{
    uint8_t i=0 , max_index=0 , max_val=0;
    //find max value
    for(i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        if(i[buff] > max_val)
        {
            max_val = i[buff];
            max_index = i;
        }
    }
    return max_index;
}


static ble_bond_err_t find_new_tag_index(uint8_t *tag)
{
    ble_bond_err_t err = BLE_BOND_SUCCESS;
    bool found_available_tag = false;
    uint8_t *age_buff = get_bond_save_age();
    //find available idle item
    for(uint8_t i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        //buff is empty
        if(age_buff[i] == 0)
        {
            *tag = i;
            found_available_tag = true;
            LOG(LOG_LVL_INFO,"found free tag=%d\n",*tag);
            break;
        }
    }
    //nvds full , delete oldest item
    if(found_available_tag == false)
    {
        *tag = get_oldest_item(age_buff);
        err = bond_save_delete_node(*tag); //delete node will set age buff to zero. 
        LOG(LOG_LVL_INFO,"not found free tag,delete oldest tag=%d\n",*tag);
    }
    //insert new item and save
    bond_save_age_insert_new_item(*tag,age_buff);
    set_bond_save_age();
    return err;
}


/**
 ****************************************************************************************
 * @brief        set the param to flash storage
 * @param[out]   id     node index to save ,must be valid id.
 * @param[in]    type   save param type
 * @param[in]    hdl    save handle
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_param_set(bond_node_id_t id , bond_type_t type , bond_handle_t hdl)
{
    ble_bond_err_t retval = BLE_BOND_SUCCESS;
    if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "bond_save_param_set:");
    if((type != BOND_SAVE_TYPE_SECURITY) && (id >= BOND_SAVE_MAX_DATABASE_CNT))
    {
        retval = BLE_BOND_ERROR_INVALID_NODE_ID;
        LOG(LOG_LVL_INFO , "BLE_BOND_ERROR_INVALID_NODE_ID,id=%d\n",id);
        return retval;
    }
    switch((uint8_t)type)
    {
        case BOND_SAVE_TYPE_SECURITY:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "SECURITY\n");
            set_security_cache_invalid();
            retval = bond_save_write(id , type , hdl.bond_security);
            break;
        case BOND_SAVE_TYPE_SIGN_COUNTER:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "SIGN_COUNTER\n");
            retval = bond_save_write(id , type , hdl.sign_counter);
            break;
        case BOND_SAVE_TYPE_DATABASE:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "DATABASE\n");
            retval = bond_save_set_database(id , hdl.bond_database);
            break;
        case BOND_SAVE_TYPE_CCCD:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "CCCD\n");
            set_cccd_cache_invalid();
            retval = bond_save_set_cccd(id , hdl.bond_cccd);
            break;
        default:
            LOG(LOG_LVL_INFO , "Unexcepted handler\n");
            retval = BLE_BOND_ERROR_INVALID_PARAM;
            break;
    }
    return retval;
}


/**
 ****************************************************************************************
 * @brief        get the param from flash storage
 * @param[in]    id     node index to get,must be valid id.
 * @param[in]    type   get param type
 * @param[inout] hdl    get/save handle
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_param_get(bond_node_id_t id , bond_type_t type , bond_handle_t hdl)
{
    ble_bond_err_t retval = BLE_BOND_SUCCESS;
    uint8_t tag;
    if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "bond_save_param_get:");
    if(id >= BOND_SAVE_MAX_DATABASE_CNT)
    {
        retval = BLE_BOND_ERROR_INVALID_NODE_ID;
        LOG(LOG_LVL_INFO , "BLE_BOND_ERROR_INVALID_NODE_ID,id=%d\n",id);
        return retval;
    }
    switch((uint8_t)type)
    {
        case BOND_SAVE_TYPE_SECURITY:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "SECURITY\n");
            retval = bond_save_read(id , type , hdl.bond_security);
            break;
        case BOND_SAVE_TYPE_SIGN_COUNTER:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "SIGN_COUNTER\n");
            retval = bond_save_read(id , type , hdl.sign_counter);
            break;
        case BOND_SAVE_TYPE_DATABASE:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "DATABASE\n");
            retval = bond_save_find_service_tag_from_id_uuid(id , hdl.bond_database->uuid , &tag);
            if(retval == BLE_BOND_SUCCESS)
            {
                retval = bond_save_read_services(tag , hdl.bond_database->length , hdl.bond_database->char_buff);
            }
            break;
        case BOND_SAVE_TYPE_CCCD:
            if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "CCCD\n");
            retval = bond_save_get_cccd_from_id_handle(id , hdl.bond_cccd->attr_handle , &hdl.bond_cccd->value);
            break;
        default:
            LOG(LOG_LVL_INFO , "Unexcepted handler\n");
            retval = BLE_BOND_ERROR_INVALID_PARAM;
            break;
    }   
    return retval;
}


/**
 ****************************************************************************************
 * @brief        Allocate a new device ID . NOT SAVE DATA. If node is full,will delete oldest node.
 * @param[out]   *id    output a valid id;
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_allocate_new_id(bond_node_id_t *id)
{
    ble_bond_err_t retval;
    retval = find_new_tag_index(id);
    return retval;
}


/**
 ****************************************************************************************
 * @brief        delete the specific node data
 * @param[in]    id     node index to save
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_delete_node(bond_node_id_t id)
{
    ble_bond_err_t retval = BLE_BOND_SUCCESS;
    ble_bond_err_t err    = BLE_BOND_SUCCESS;
    
    bond_security_t             info;
    bond_database_idx_store_t   db;
    uint8_t *age_buff;
    
    LOG(LOG_LVL_INFO , "delete node(%d):",id);
    //judge valid
    if(id >= BOND_SAVE_MAX_DATABASE_CNT)
    {
        LOG(LOG_LVL_WARN , "BLE_BOND_ERROR_INVALID_NODE_ID,id=%d\n",id);
        return BLE_BOND_ERROR_INVALID_NODE_ID;
    }
    //1.update age buff
    age_buff = get_bond_save_age();
    age_buff[id] = 0;
    set_bond_save_age();
    //2.delete security
    err = bond_save_read(id , BOND_SAVE_TYPE_SECURITY , &info);
    if(BLE_BOND_SUCCESS == err)
    {
        bond_save_delete_tag(BOND_SAVE_DEVICE_TO_TAG_SECURITY(id)); 
        LOG(LOG_LVL_INFO , "del security\t");
    }
    else
    {
        LOG(LOG_LVL_INFO , "no security\t");
    }
    //3.delete database
    err = bond_save_read(id , BOND_SAVE_TYPE_DATABASE , &db  ); 
    if(err == BLE_BOND_SUCCESS)
    {
        //3.1 delete database index tag.
        bond_save_delete_tag(BOND_SAVE_DEVICE_TO_TAG_DATABASE(id)); 
        //3.2 delete services tags and usage buff
        //If we call "bond_save_delete_services_and_usage" this will cause write multi times of db_usage.
        //If we call "bond_save_delete_multi_services_and_usage",will only write db_usage once.
        retval = bond_save_delete_multi_services_and_usage(&db);
        LOG(LOG_LVL_INFO , "del database\t");
    }
    else
    {
        LOG(LOG_LVL_INFO , "no database\t");
    }
    //4.delete sign_counter
    if(BLE_BOND_SUCCESS == bond_save_delete_tag(BOND_SAVE_DEVICE_TO_TAG_SIGNCNT(id)))
    {
        LOG(LOG_LVL_INFO , "del sign_cnt\t");
    }
    else
    {
        LOG(LOG_LVL_INFO , "no sign_cnt\t");
    }
    //5.delete cccd
    if(BLE_BOND_SUCCESS == bond_save_delete_tag(BOND_SAVE_DEVICE_TO_TAG_CCCD(id)))
    {
        LOG(LOG_LVL_INFO , "del cccd...\n");
    }       
    else
    {
        LOG(LOG_LVL_INFO , "no cccd...\n");
    }    
    return retval;
}

/**
 ****************************************************************************************
 * @brief        delete all the node data
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_delete_all(void)
{
    ble_bond_err_t retval = BLE_BOND_SUCCESS;
    ble_bond_err_t err    = BLE_BOND_SUCCESS;
    uint8_t *age_buff , i;
    
    LOG(LOG_LVL_INFO , "bond_save_delete_all\n");
    //get buff
    age_buff = get_bond_save_age();
    //delete buff [0 ~ (BOND_SAVE_MAX_DEVICE_CNT-1)]
    i = BOND_SAVE_MAX_DEVICE_CNT;
    while(i --> 0)
    {
        //delete age buff
        if(age_buff[i] > 0)
        {
            //delete ndoe and delay for log print.
            err = bond_save_delete_node(i);
            BX_DELAY_US(1*1000);
            //error handle
            if(err != NVDS_OK)
            {
                LOG(LOG_LVL_WARN , "bond_save_delete_all,err=%d\n",err);
                retval = err;
            }
        }
    }
    return retval;
}





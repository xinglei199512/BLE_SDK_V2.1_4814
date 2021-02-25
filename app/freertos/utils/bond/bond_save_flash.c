/**
 ****************************************************************************************
 *
 * @file   bond_save_flash.c
 *
 * @brief  ble bond save to nvds flash.
 *
 * @author  Chen Jia Chuang
 * @date    2018-12-26
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
#include "nvds_in_ram.h"
#include "string.h"
#include "log.h"
#include "osapp_config.h"
#include "osapp_task.h"
#include "bond_save_db_cccd.h"
#include "bond_save_security.h"





/*
 * LOCAL VRAIABLE
 ****************************************************************************************
 */
KEIL_STATIC uint8_t bound_save_age_buff     [NVDS_LEN_BOND_SAVE_AGE]       ={0};
//   a[0]       a[1]
//LSB...MSB  LSB...MSB
//  0...7      8...15
KEIL_STATIC uint8_t bound_save_db_usage_buff        [NVDS_LEN_BOND_SAVE_DB_USAGE]  ={0};



/*
 * export api
 ****************************************************************************************
 */
uint8_t *get_bound_save_db_usage_buff(void)
{
    return bound_save_db_usage_buff;
}


/*
 * basic set get
 ****************************************************************************************
 */
static void bond_save_get_len_tag(uint8_t index , bond_type_t type , nvds_tag_len_t *length , uint8_t *tag)
{
    switch((uint8_t)type)
    {
        case BOND_SAVE_TYPE_SECURITY:
            *length = NVDS_LEN_BOND_SAVE_SECURITY;
            *tag    = BOND_SAVE_DEVICE_TO_TAG_SECURITY(index);
            break;
        case BOND_SAVE_TYPE_SIGN_COUNTER:
            *length = NVDS_LEN_BOND_SAVE_SIGN_COUNTER;
            *tag    = BOND_SAVE_DEVICE_TO_TAG_SIGNCNT(index);
            break;
        case BOND_SAVE_TYPE_DATABASE:
            *length = NVDS_LEN_BOND_SAVE_DATABASE_IDX;
            *tag    = BOND_SAVE_DEVICE_TO_TAG_DATABASE(index);
            break;
        case BOND_SAVE_TYPE_CCCD:
            *length = NVDS_LEN_BOND_SAVE_CCCD;
            *tag    = BOND_SAVE_DEVICE_TO_TAG_CCCD(index);
            break;
        default:
            LOG(LOG_LVL_INFO , "Unexcepted handler\n");
            *length = 0;
            *tag    = 0;
            break;
    }
}
void bond_save_get_real_length(nvds_tag_len_t *length , const bond_database_idx_store_t *buff)
{
    *length = sizeof(uint8_t) + sizeof(bond_database_idx_t) * buff->length;
}
ble_bond_err_t bond_save_read(bond_node_id_t index , bond_type_t type , void *buff)
{
    nvds_tag_len_t length;
    uint8_t tag;
    
    bond_save_get_len_tag(index , type , &length , &tag);
    memset(buff , 0x00 , length);
    uint8_t nvds_err = nvds_get(tag , &length , (void*)buff);
    if(nvds_err != NVDS_OK)
    {
        //LOG(LOG_LVL_WARN , "bond_save_read,nvds_err=%d\n",nvds_err);
    }
    if(nvds_err == NVDS_LENGTH_OUT_OF_RANGE) return BLE_BOND_ERROR_NVDS_LENGTH_ERR;
    if(nvds_err == NVDS_TAG_NOT_DEFINED    ) return BLE_BOND_ERROR_NOT_FOUND;
    return BLE_BOND_SUCCESS;
}
ble_bond_err_t bond_save_write(bond_node_id_t index , bond_type_t type , void* buff)
{
    nvds_tag_len_t length;
    uint8_t tag;
    
    bond_save_get_len_tag(index , type , &length , &tag);
    //if this is the database index , will not necessary to use entire array.Only write valid part of array.
    if(type == BOND_SAVE_TYPE_DATABASE)
    {
        bond_save_get_real_length(&length , buff);
    }
    uint8_t nvds_err = nvds_put(tag, length , (void*)buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "bond_save_write,nvds_err=%d\n",nvds_err);
    }
    return (nvds_err == NVDS_OK) ? BLE_BOND_SUCCESS : BLE_BOND_ERROR_NVDS_WRITE_ERR;
}

/*
 * services
 ****************************************************************************************
 */
ble_bond_err_t bond_save_read_services(uint8_t tag , uint8_t length , uint8_t *buff)
{
    uint8_t len = length;
    uint8_t nvds_err = nvds_get(tag , &len, (void*)buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "bond_save_read_services,nvds_err=%d\n",nvds_err);
    }
    return (nvds_err == NVDS_OK) ? BLE_BOND_SUCCESS : BLE_BOND_ERROR_NOT_FOUND;
}
ble_bond_err_t bond_save_write_services_and_usage(uint8_t tag , uint8_t length , uint8_t *buff , bool add_usage)
{
    if(add_usage)
    {
        //save services uasge mask
        bond_save_db_usage_get();
        uint8_t i,b,m;
        i = tag - NVDS_TAG_BOND_GATT_DB_START;
        b = i / 8;
        m = i % 8;
        b[bound_save_db_usage_buff] |= (1 << m);
        bond_save_db_usage_set();
    }
    //save services 
    uint8_t nvds_err = nvds_put(tag, length , (void*)buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "bond_save_write_services_and_usage,nvds_err=%d\n",nvds_err);
    }
    return (nvds_err == NVDS_OK) ? BLE_BOND_SUCCESS : BLE_BOND_ERROR_NVDS_WRITE_ERR;
}
ble_bond_err_t bond_save_delete_services_and_usage(uint8_t tag) //Not used now.
{
    //save services uasge mask
    bond_save_db_usage_get();
    uint8_t i,b,m;
    i = tag - NVDS_TAG_BOND_GATT_DB_START;
    b = i / 8;
    m = i % 8;
    b[bound_save_db_usage_buff] &= ~(1 << m);
    bond_save_db_usage_set();
    //delete services
    uint8_t nvds_err = nvds_del(tag);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "bond_save_delete_services_and_usage,nvds_err=%d\n",nvds_err);
    }
    return (nvds_err == NVDS_OK) ? BLE_BOND_SUCCESS : BLE_BOND_ERROR_NVDS_DELETE_ERR;
}
ble_bond_err_t bond_save_delete_multi_services_and_usage(bond_database_idx_store_t *db)
{
    uint8_t i,b,m,cnt,tag;
    uint8_t nvds_err,retval;
    retval = BLE_BOND_SUCCESS;
    //get usage
    bond_save_db_usage_get();
    //delete tags
    for(cnt=0 ; cnt < db->length ; cnt ++)
    {
        //get tag
        tag = db->array[cnt].nvds_tag;
        //set usage buffer
        i = tag - NVDS_TAG_BOND_GATT_DB_START;
        b = i / 8;
        m = i % 8;
        b[bound_save_db_usage_buff] &= ~(1 << m);
        //delete services
        nvds_err = nvds_del(tag);
        //error handle
        if(nvds_err != NVDS_OK)
        {
            LOG(LOG_LVL_WARN , "bond_save_delete_multi_services_and_usage,nvds_err=%d\n",nvds_err);
            retval = nvds_err;
        }
    }
    //set usage
    bond_save_db_usage_set();
    return retval;
}


/*
 * age
 ****************************************************************************************
 */
uint8_t *get_bond_save_age(void)
{
    //init value
    nvds_tag_len_t length = NVDS_LEN_BOND_SAVE_AGE;
    uint8_t tag   = NVDS_TAG_BOND_SAVE_AGE;
    uint8_t *buff = bound_save_age_buff;
    //operation
    memset(buff , 0x00 , NVDS_LEN_BOND_SAVE_AGE);
    uint8_t nvds_err = nvds_get(tag , &length , (void*)buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "get_bond_save_age,nvds_err=%d\n",nvds_err);
    }
    //return 
    return buff;
}
void set_bond_save_age(void)
{
    //init value
    uint8_t tag   = NVDS_TAG_BOND_SAVE_AGE;
    uint8_t *buff = bound_save_age_buff;
    //operation
    uint8_t nvds_err = nvds_put(tag , NVDS_LEN_BOND_SAVE_AGE , (void*)buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "set_bond_save_age,nvds_err=%d\n",nvds_err);
    }
}

/*
 * db usage
 ****************************************************************************************
 */
void bond_save_db_usage_get(void)
{
    nvds_tag_len_t length = NVDS_LEN_BOND_SAVE_DB_USAGE;
    uint8_t nvds_err = nvds_get(NVDS_TAG_BOND_SAVE_DB_TAG_USED , &length , (void*)bound_save_db_usage_buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "bond_save_db_usage_get,nvds_err=%d\n",nvds_err);
    }
}
void bond_save_db_usage_set(void)
{
    nvds_tag_len_t length = NVDS_LEN_BOND_SAVE_DB_USAGE;
    uint8_t nvds_err = nvds_put(NVDS_TAG_BOND_SAVE_DB_TAG_USED , length , (void*)bound_save_db_usage_buff);
    if(nvds_err != NVDS_OK)
    {
        LOG(LOG_LVL_WARN , "bond_save_db_usage_set,nvds_err=%d\n",nvds_err);
    }
}

/*
 * tags
 ****************************************************************************************
 */
ble_bond_err_t bond_save_delete_tag(uint8_t tag)
{
    uint8_t nvds_err = nvds_del(tag);
    if(nvds_err != NVDS_OK)
    {
        //LOG(LOG_LVL_WARN , "bond_save_delete_tag,nvds_err=%d\n",nvds_err);
    }
    return (nvds_err == NVDS_OK) ? BLE_BOND_SUCCESS : BLE_BOND_ERROR_NVDS_DELETE_ERR;
}

/**
 ****************************************************************************************
 * @brief        Init bond save module.
 * @return       None.
 ****************************************************************************************
 */
void bond_save_init(void)
{
    nvds_tag_len_t length;
    uint8_t nvds_err;
    memset(bound_save_age_buff      , 0 , NVDS_LEN_BOND_SAVE_AGE     );
    memset(bound_save_db_usage_buff , 0 , NVDS_LEN_BOND_SAVE_DB_USAGE);
    //invalid cache
    set_security_cache_invalid();
    set_cccd_cache_invalid();
    //age buff
    length = NVDS_LEN_BOND_SAVE_AGE;
    nvds_err = nvds_get(NVDS_TAG_BOND_SAVE_AGE , &length , (void*)bound_save_age_buff);
    if(nvds_err == NVDS_TAG_NOT_DEFINED)
    {
        nvds_err = nvds_put(NVDS_TAG_BOND_SAVE_AGE , length , (void*)bound_save_age_buff);
        LOG(LOG_LVL_INFO , "FIRST ADD NVDS_TAG_BOND_SAVE_AGE_CENTRAL,nvds_err=%d\n",nvds_err);
    }
    //usage
    length = NVDS_LEN_BOND_SAVE_DB_USAGE;
    nvds_err = nvds_get(NVDS_TAG_BOND_SAVE_DB_TAG_USED , &length , (void*)bound_save_db_usage_buff);
    if(nvds_err == NVDS_TAG_NOT_DEFINED)
    {
        nvds_err = nvds_put(NVDS_TAG_BOND_SAVE_DB_TAG_USED , length , (void*)bound_save_db_usage_buff);
        LOG(LOG_LVL_INFO , "FIRST ADD NVDS_TAG_BOND_SAVE_DB_TAG_USED,nvds_err=%d\n",nvds_err);
    }
}


/**
 ****************************************************************************************
 * @brief        write to file system.
 * @return       None.
 ****************************************************************************************
 */
void bond_save_write_through(void)
{
    nvds_write_through();
}










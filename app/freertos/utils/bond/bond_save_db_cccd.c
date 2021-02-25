/**
 ****************************************************************************************
 *
 * @file   bond_save_db_cccd.c
 *
 * @brief  ble bond save database and cccd.
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
#include "bond_save_db_cccd.h"


/*
 * TYPE DEFINE
 ****************************************************************************************
 */
typedef struct
{
    uint8_t node_id;        //node index
    bool    cache_valid;    //this cache is valid
    bond_cccd_store_t cccd_store;
} cccd_cache_t;



/*
 * LOCAL VRAIABLE
 ****************************************************************************************
 */
KEIL_STATIC cccd_cache_t cccd_cache;
/*
 * DATABASE
 ****************************************************************************************
 */
//internal valid mask
static ble_bond_err_t bond_save_find_new_services_tag(uint8_t *tag)
{
    uint8_t i,b,m;
    ble_bond_err_t err = BLE_BOND_ERROR_SERVICES_FULL;
    uint8_t *usage_buff = get_bound_save_db_usage_buff();
    bond_save_db_usage_get();
    for(i=0 ; i<BOND_SAVE_MAX_DB_SIZE ;i++)
    {
        b = i / 8;
        m = i % 8;
        if((b[usage_buff] & (1 << m)) == 0)
        {
            *tag = i + NVDS_TAG_BOND_GATT_DB_START;
            err = BLE_BOND_SUCCESS;
            break;
        }
    }
    return err;
}


ble_bond_err_t bond_save_set_database(bond_node_id_t id , bond_database_t *db)
{
    ble_bond_err_t err = BLE_BOND_SUCCESS;
    bool uuid_is_exist = false;
    uint8_t services_tag;
    bond_database_idx_store_t db_store_idx;
    //read index
    if(BLE_BOND_SUCCESS == bond_save_read(id , BOND_SAVE_TYPE_DATABASE , &db_store_idx))
    {
        //find if this uuid is exist
        for(uint8_t i=0 ; i < db_store_idx.length ; i++)
        {
            if(memcmp(db_store_idx.array[i].uuid , db->uuid , 16) == 0)
            {
                uuid_is_exist = true;
                services_tag = db_store_idx.array[i].nvds_tag;
                //don't need save idx
                //save services
                err = bond_save_write_services_and_usage(services_tag , db->length , db->char_buff , false);
                if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO,"find this uuid is exist,id=%d,services_tag=%d\n",id,services_tag);
                break;
            }
        }
    }
    //if empty then insert new item
    if(uuid_is_exist == false)
    {
        //if services is not full
        if(db_store_idx.length != BOND_SAVE_MAX_DATABASE_CNT)
        {
            //find new services tag
            err = bond_save_find_new_services_tag(&services_tag);
            //if already have available tags
            if(err == BLE_BOND_SUCCESS)
            {
                //save idx
                db_store_idx.length ++;
                db_store_idx.array[db_store_idx.length-1].nvds_tag = services_tag;
                memcpy(db_store_idx.array[db_store_idx.length-1].uuid , db->uuid , 16);
                err = bond_save_write(id , BOND_SAVE_TYPE_DATABASE , &db_store_idx);
                if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO,"Add new uuid database,id=%d,services_tag=%d\n",id,services_tag);
                //save services
                if(err == BLE_BOND_SUCCESS)
                {
                    err = bond_save_write_services_and_usage(services_tag , db->length , db->char_buff , true);
                }
            }
            else
            {
                LOG(LOG_LVL_WARN , "BLE_BOND_ERROR_SERVICES_FULL\n");
            }
        } 
        else
        {
            err = BLE_BOND_ERROR_DATABASE_INDEX_TAG_FULL;
            LOG(LOG_LVL_WARN , "BLE_BOND_ERROR_DATABASE_INDEX_TAG_FULL\n");
        }    
    }
    return err;
}

ble_bond_err_t bond_save_find_service_tag_from_id_uuid(bond_node_id_t id , uint8_t *uuid , uint8_t *tag)
{
    ble_bond_err_t err = BLE_BOND_ERROR_NOT_FOUND;
    bond_database_idx_store_t db_store_idx;
    //read index
    bond_save_read(id , BOND_SAVE_TYPE_DATABASE , &db_store_idx);
    //find if this uuid is exist
    for(uint8_t i=0 ; i < db_store_idx.length ; i++)
    {
        if(memcmp(db_store_idx.array[i].uuid , uuid , 16) == 0)
        {
            *tag = db_store_idx.array[i].nvds_tag;
            err = BLE_BOND_SUCCESS;
            break;
        }
    }
    return err;
}

/*
 * CCCD
 ****************************************************************************************
 */

static void read_cccd_to_cache(bond_node_id_t id)
{
    bond_save_read(id , BOND_SAVE_TYPE_CCCD , &cccd_cache.cccd_store);
    cccd_cache.cache_valid = true;
    cccd_cache.node_id = id;
}
void set_cccd_cache_invalid(void)
{
    cccd_cache.cache_valid = false;
}

ble_bond_err_t bond_save_set_cccd(bond_node_id_t id , bond_cccd_t *cccd)
{
    ble_bond_err_t err = BLE_BOND_SUCCESS;
    bool item_is_exist = false;
    //read index
    read_cccd_to_cache(id);
    //find if this item is exist
    for(uint8_t i=0 ; i < cccd_cache.cccd_store.length ; i++)
    {
        if(cccd_cache.cccd_store.array[i].attr_handle == cccd->attr_handle)
        {
            item_is_exist = true;
            //save cccd
            if(cccd_cache.cccd_store.array[i].value != cccd->value)    //value is different , update value.
            {
                cccd_cache.cccd_store.array[i].value = cccd->value;
                err = bond_save_write(id , BOND_SAVE_TYPE_CCCD , &cccd_cache.cccd_store);
            }
            else
            {
                //value is same , done't need update.
                err = BLE_BOND_SUCCESS;
            }
            break;
        }
    }
    //add new item
    if(item_is_exist == false)
    {
        //if services is not full
        if(cccd_cache.cccd_store.length < BOND_SAVE_MAX_DATABASE_CNT)
        {
            cccd_cache.cccd_store.length ++;
            cccd_cache.cccd_store.array[cccd_cache.cccd_store.length - 1].value       = cccd->value;
            cccd_cache.cccd_store.array[cccd_cache.cccd_store.length - 1].attr_handle = cccd->attr_handle;
            err = bond_save_write(id , BOND_SAVE_TYPE_CCCD , &cccd_cache.cccd_store);
        } 
        else
        {
            err = BLE_BOND_ERROR_DATABASE_INDEX_TAG_FULL;
            LOG(LOG_LVL_WARN , "BLE_BOND_ERROR_DATABASE_INDEX_TAG_FULL\n");
        }  
    }
    return err;
}

//id[in],attr_handle[in],*value[out]
ble_bond_err_t bond_save_get_cccd_from_id_handle(uint8_t id , uint16_t attr_handle , uint16_t *value)
{
    ble_bond_err_t err = BLE_BOND_ERROR_NOT_FOUND;
    //if in cache and valid , use cache value.
    if((cccd_cache.node_id == id) && (true == cccd_cache.cache_valid))
    {
        //USE CACHE HERE.
        if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "get cccd from cache.\n");
    }
    else//if not in cache , read the data from nvds
    {
        read_cccd_to_cache(id);
    }
    //find if this item is exist
    for(uint8_t i=0 ; i < cccd_cache.cccd_store.length ; i++)
    {
        if(cccd_cache.cccd_store.array[i].attr_handle == attr_handle)
        {
            *value = cccd_cache.cccd_store.array[i].value;
            err = BLE_BOND_SUCCESS;
            break;
        }
    }
    return err;
}



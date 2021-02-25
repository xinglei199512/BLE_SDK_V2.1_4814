/**
 ****************************************************************************************
 *
 * @file   bond_save_security.c
 *
 * @brief  ble bond save apis.
 *
 * @author  Chen Jia Chuang
 * @date    2018-12-25
 * @version <0.0.0.1>
 *
 * @license
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */
#include "bond_save.h"
#include "bond_save_security.h"
#include "bond_save_flash.h"
#include "string.h"
#include "log.h"
#include "bond_manage.h"


/*
 * TYPE DEFINE
 ****************************************************************************************
 */
typedef struct
{
    uint8_t node_id;        //node index
    bool    cache_valid;    //this cache is valid
    bond_security_t security_store;
} security_cache_t;



/*
 * LOCAL VRAIABLE
 ****************************************************************************************
 */
//recover temp info
KEIL_STATIC bond_recover_cb_t    bond_recover_callback;
//recover cache
KEIL_STATIC security_cache_t     security_cache;

/*
 * FUNCTION DECLEAR
 ****************************************************************************************
 */
static void bond_save_recover_ltk(uint8_t index);



/*
 * BOND SECURITY CACHE
 ****************************************************************************************
 */
static ble_bond_err_t read_security_to_cache(bond_node_id_t id)
{
    ble_bond_err_t err;
    err = bond_save_read(id , BOND_SAVE_TYPE_SECURITY , &security_cache.security_store);
    security_cache.cache_valid = true;
    security_cache.node_id = id;
    return err;
}
void set_security_cache_invalid(void)
{
    security_cache.cache_valid = false;
}
 

/*  
 * BOND IRK MATCH
 ****************************************************************************************
 */

static void find_index_from_irk(const uint8_t* irk , uint8_t *index)
{
    uint8_t *buff = get_bond_save_age();
    for(uint8_t i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        if(buff[i] > 0)
        {
            if(BLE_BOND_SUCCESS == read_security_to_cache(i))
            {
                if(memcmp(irk , security_cache.security_store.info.irk.key , GAP_KEY_LEN) == 0)
                {
                    *index = i;
                    break;
                }
            }
        }
    }
}

static void bond_save_irk_match_finish_cb(bool status,struct gapm_addr_solved_ind const *param)
{
    uint8_t index;
    if(status == 1)
    {
        find_index_from_irk(param->irk.key , &index);
        LOG(LOG_LVL_INFO , "IRK matched success!!! index=%d\n",index);
        bond_save_recover_ltk(index);
    }
    else
    {
        LOG(LOG_LVL_WARN,"IRK matched fail!!!\n");
        bond_recover_callback(0 , BOND_NODE_UNALLOC_ID);
    }
}
static void bond_save_irk_match_data_cb (struct gap_sec_key *irk)
{
    uint8_t index = 0;
    uint8_t *buff = get_bond_save_age();
    for(uint8_t i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        if(buff[i] > 0)
        {
            if(BLE_BOND_SUCCESS == read_security_to_cache(i))
            {
                index[irk] = security_cache.security_store.info.irk;
                index ++;
            }
        }
    }
}

static void find_exist_random_addr_index(bd_addr_t device_addr)
{
    uint8_t key_cnt = 0;
    //calc number of irk
    uint8_t *buff = get_bond_save_age();
    for(uint8_t i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        if(buff[i] > 0)
        {
            key_cnt ++;
        }
    }
    LOG(LOG_LVL_INFO , "total available key cnt=%d\n",key_cnt);
    //recover
    if(key_cnt > 0)
    {
        bond_manage_random_resolve_private_addr(device_addr , key_cnt , bond_save_irk_match_data_cb , bond_save_irk_match_finish_cb);
    }
    else
    {
        bond_recover_callback(false , BOND_NODE_UNALLOC_ID);
    }
}

/* 
 * BOND PUBLIC KEY MATCH
 ****************************************************************************************
 */
static ble_bond_err_t find_exist_public_addr_index(uint8_t* addr , uint8_t *index)
{
    ble_bond_err_t err = BLE_BOND_ERROR_NOT_FOUND;
    uint8_t *buff = get_bond_save_age();
    for(uint8_t i=0;i<BOND_SAVE_MAX_DEVICE_CNT;i++)
    {
        if(buff[i] > 0)
        {
            ble_bond_err_t nvds_state = read_security_to_cache(i);
            if((nvds_state == BLE_BOND_SUCCESS) && (memcmp(addr , security_cache.security_store.bdaddr.addr.addr , GAP_BD_ADDR_LEN) == 0))
            {
                *index = i;
                err = BLE_BOND_SUCCESS;
                break;
            }
        }
    }
    return err;
}



/* 
 * BOND RECOVER 
 ****************************************************************************************
 */
static void bond_save_recover_ltk(uint8_t index)
{
    LOG(LOG_LVL_INFO , "bond_save_recover_ltk success\n");
    bond_recover_callback(true , index);
}
static bool match_device_from_addr(struct gap_bdaddr *bdaddr)   //public address or static random address
{
    bool retval = true;
    if((bdaddr->addr_type == ADDR_RAND) && ((bdaddr->addr.addr[5] & 0xC0) == GAP_RSLV_ADDR))
    {
        retval = false;
    }
    return retval;
}


/**
 ****************************************************************************************
 * @brief        recover the security data from BD address
 * @param[in]    info           input address
 * @param[in]    bond_recover   complete callback.
 * @return       None.
 ****************************************************************************************
 */
void bond_save_recover_security(struct gap_bdaddr *bdaddr , bond_recover_cb_t bond_recover_cb)
{
    uint8_t index;
    bond_recover_callback   = bond_recover_cb;
    //find index from addr or irk
    if(match_device_from_addr(bdaddr))
    {
        LOG(LOG_LVL_INFO , "bond_save_recover_from_bd_addr -> public addr\n");
        if(BLE_BOND_SUCCESS == find_exist_public_addr_index(bdaddr->addr.addr , &index))
        {
            bond_save_recover_ltk(index);
        }
        else
        {
            bond_recover_callback(false , BOND_NODE_UNALLOC_ID);
        }
    }
    else
    {
        LOG(LOG_LVL_INFO , "bond_save_recover_from_bd_addr -> random addr\n");
        find_exist_random_addr_index(bdaddr->addr);
    }
}


/**
 ****************************************************************************************
 * @brief        get the security param from flash storage
 * @param[in]    id     node index to get,must be valid id.
 * @param[in]    type   get security param type
 * @param[out]   buff   output content
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_security_get(bond_node_id_t id , security_type_t type , void* buff)
{
    //valid judge
    ble_bond_err_t retval = BLE_BOND_SUCCESS;
    if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "bond_save_security_get:");
    if(id >= BOND_SAVE_MAX_DATABASE_CNT)
    {
        retval = BLE_BOND_ERROR_INVALID_NODE_ID;
        LOG(LOG_LVL_INFO , "BLE_BOND_ERROR_INVALID_NODE_ID,id=%d\n",id);
        return retval;
    }
    //if in cache and valid , use cache value.
    if((security_cache.node_id == id) && (true == security_cache.cache_valid))
    {
        //USE CACHE HERE.
        if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "from cache.\n");
    }
    else//if not in cache , read the data from nvds
    {
        //READ FROM NVDS
        if(BOND_SAVE_OPEN_NORMAL_LOG) LOG(LOG_LVL_INFO , "from nvds.\n");
        retval = read_security_to_cache(id);
    }
    //switch types
    if(retval == BLE_BOND_SUCCESS)
    {
        switch((uint8_t)type)
        {
            case BONDSAVE_SECURITY_LTK_LOCAL:
                if(security_cache.security_store.info.ltk_present)
                {
                    memcpy(buff , &security_cache.security_store.info.ltk_local  , sizeof(struct gapc_ltk));
                }
                else
                {
                    retval = BLE_BOND_ERROR_NOT_FOUND;
                }
                break;
            case BONDSAVE_SECURITY_LTK_PEER:
                if(security_cache.security_store.info.ltk_present)
                {
                    memcpy(buff , &security_cache.security_store.info.ltk_peer   , sizeof(struct gapc_ltk));
                }
                else
                {
                    retval = BLE_BOND_ERROR_NOT_FOUND;
                }
                break;
            case BONDSAVE_SECURITY_CSRK_LOCAL:
                if(security_cache.security_store.info.csrk_present)
                {
                    memcpy(buff , &security_cache.security_store.info.csrk_local , sizeof(struct gap_sec_key));
                }
                else
                {
                    retval = BLE_BOND_ERROR_NOT_FOUND;
                }
                break;
            case BONDSAVE_SECURITY_CSRK_PEER:
                if(security_cache.security_store.info.csrk_present)
                {
                    memcpy(buff , &security_cache.security_store.info.csrk_peer  , sizeof(struct gap_sec_key));
                }
                else
                {
                    retval = BLE_BOND_ERROR_NOT_FOUND;
                }
                break;
            case BONDSAVE_SECURITY_IRK:
                if(security_cache.security_store.info.irk_present)
                {
                    memcpy(buff , &security_cache.security_store.info.irk        , sizeof(struct gap_sec_key));
                }
                else
                {
                    retval = BLE_BOND_ERROR_NOT_FOUND;
                }
                break;
            case BONDSAVE_SECURITY_AUTH:
                memcpy(buff , &security_cache.security_store.info.auth       , sizeof(uint8_t));
                break;
            default:
                LOG(LOG_LVL_INFO , "Unexcepted handler\n");
                retval = BLE_BOND_ERROR_INVALID_PARAM;
                break;
        }   
    }
    return retval;
}











/**
 ****************************************************************************************
 *
 * @file   bond_save.h
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

#ifndef _BOND_SAVE_
#define _BOND_SAVE_
#include "gap.h"
#include "gapm_task.h"
#include "gapc_task.h"
#include "stdbool.h"
#include "co_bt_defines.h"
#include "nvds_typedef.h"
#include "ble_bond_errors.h"



/*
 * NOTE!!!
 ****************************************************************************************
 * Must make sure nvds space and tags available.
 * change macro NVDS_TAG_TABLE_MAX  to 251 
 * change macro NVDS_DATA_SECTION_MAX_SIZE to 0x4000 (16KByte)
 * change file merge.ini nvds_size to 0x4000 (16KByte)
 *
 */
 
 
 
/*
 * DEFINE
 ****************************************************************************************
 */
//MAX VAL
#define BOND_SAVE_MAX_DATABASE_CNT          10      //max database count per device 
#define BOND_SAVE_MAX_CCCD_CNT              10      //max cccd count per device
#define BOND_SAVE_MAX_DEVICE_CNT            10      //max support device,less than : (NVDS_TAG_BOND_DEV_INFO_END-NVDS_TAG_BOND_DEV_INFO_START+1)/4

//CONST VAL
#define BOND_NODE_UNALLOC_ID                0xFF    //new device unalloc device number
#define KEIL_STATIC                                 //for keil debug

//MACROS
#define BOND_SAVE_OPEN_NORMAL_LOG           0       //if 1 then open the normal operation log. If 0 wil disable normal log , but will not disable important log


/*
 * TYPE DEFINE
 ****************************************************************************************
 */
/// security information
typedef struct 
{
    /// store information related to link key (local)
    struct gapc_ltk ltk_local;
    /// store information related to link key (peer)
    struct gapc_ltk ltk_peer;
    
    /// Local CSRK value
    struct gap_sec_key csrk_local;
    /// Remote CSRK value
    struct gap_sec_key csrk_peer;
    
    /// Identity Resolving Key
    struct gap_sec_key irk;
    
    /// Authentication (@see gap_auth)
    uint8_t auth;
        
    uint8_t ltk_present  : 1,
            csrk_present : 1,
            irk_present  : 1;
}bond_security_info_t; 

/// bond security [set/get] type
typedef struct 
{
    bond_security_info_t    info;   //set:[in]  get:[out]
    struct gap_bdaddr       bdaddr; //set:[in]  get:[in]
}bond_security_t; 



/// sign counter [set/get/store] type
typedef struct
{
    uint32_t local;     //set:[in]  get:[out]
    uint32_t peer;      //set:[in]  get:[out]
}sign_counter_t;


/// database [set/get] type
typedef struct
{
    uint8_t uuid[16];   //set:[in]  get:[in]
    uint8_t length;     //set:[in]  get:[in]
    uint8_t *char_buff; //set:[in]  get:[out]
}bond_database_t;

/// cccd [set/get] type
typedef struct
{
    uint16_t attr_handle;   //set:[in]  get:[in]
    uint16_t value;         //set:[in]  get:[out]
}bond_cccd_t;


/// handler type
typedef union
{
    bond_security_t     *bond_security;     
    sign_counter_t      *sign_counter;
    bond_database_t     *bond_database;
    bond_cccd_t         *bond_cccd;
} bond_handle_t;


/// bond_type type
typedef enum
{
    BOND_SAVE_TYPE_SECURITY     = 0,
    BOND_SAVE_TYPE_SIGN_COUNTER = 1,
    BOND_SAVE_TYPE_DATABASE     = 2,
    BOND_SAVE_TYPE_CCCD         = 3,
}bond_type_t;

//read security 
typedef enum
{
    BONDSAVE_SECURITY_LTK_LOCAL,
    BONDSAVE_SECURITY_LTK_PEER,
    BONDSAVE_SECURITY_CSRK_LOCAL,
    BONDSAVE_SECURITY_CSRK_PEER,
    BONDSAVE_SECURITY_IRK,
    BONDSAVE_SECURITY_AUTH,
}security_type_t;





/// the node unique save index type , range is 0 ~ (BOND_SAVE_MAX_DATABASE_CNT-1)
typedef uint8_t bond_node_id_t;


/// recover status callback type
typedef void (*bond_recover_cb_t)(bool success , bond_node_id_t id);


/*
 * FUNCTION DECLEAR
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief        recover the security data from BD address
 * @param[in]    info           input address
 * @param[in]    bond_recover   complete callback.
 * @return       None.
 ****************************************************************************************
 */
void bond_save_recover_security(struct gap_bdaddr *bdaddr , bond_recover_cb_t bond_recover_cb);



/**
 ****************************************************************************************
 * @brief        set the param to flash storage
 * @param[in]    id     node index to save ,must be valid id.
 * @param[in]    type   save param type
 * @param[in]    hdl    save handle
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_param_set(bond_node_id_t id , bond_type_t type , bond_handle_t hdl);

/**
 ****************************************************************************************
 * @brief        get the param from flash storage
 * @param[in]    id     node index to get,must be valid id.
 * @param[in]    type   get param type
 * @param[inout] hdl    get/save handle.
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_param_get(bond_node_id_t id , bond_type_t type , bond_handle_t hdl);

/**
 ****************************************************************************************
 * @brief        Allocate a new device ID . NOT SAVE DATA. If node is full,will delete oldest node.
 * @param[out]   *id    output a valid id;
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_allocate_new_id(bond_node_id_t *id);

/**
 ****************************************************************************************
 * @brief        delete the specific node data
 * @param[in]    id     node index to save
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_delete_node(bond_node_id_t id);

/**
 ****************************************************************************************
 * @brief        delete all the node data
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_delete_all(void);


/**
 ****************************************************************************************
 * @brief        Init bond save module.
 * @return       None.
 ****************************************************************************************
 */
void bond_save_init(void);


/**
 ****************************************************************************************
 * @brief        get the security param from flash storage
 * @param[in]    id     node index to get,must be valid id.
 * @param[in]    type   get security param type
 * @param[out]   buff   output content
 * @return       Error number.
 ****************************************************************************************
 */
ble_bond_err_t bond_save_security_get(bond_node_id_t id , security_type_t type , void* buff);


/**
 ****************************************************************************************
 * @brief        write to file system.
 * @return       None.
 ****************************************************************************************
 */
void bond_save_write_through(void);



#endif







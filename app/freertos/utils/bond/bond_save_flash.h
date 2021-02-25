/**
 ****************************************************************************************
 *
 * @file   bond_save_flash.h
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


/*
 * TYPE DEFINE
 ****************************************************************************************
 */
/// database idx [store] type
typedef struct
{
    uint8_t uuid[16];
    uint8_t nvds_tag;
}bond_database_idx_t;
typedef struct
{
    uint8_t length;
    bond_database_idx_t array[BOND_SAVE_MAX_DATABASE_CNT];
}bond_database_idx_store_t;

/// cccd idx [store] type
typedef struct
{
    uint16_t length;
    bond_cccd_t array[BOND_SAVE_MAX_CCCD_CNT];
}bond_cccd_store_t;

/// services [store] type : uint8_t*

/*
 * CONST DEFINE
 ****************************************************************************************
 */
//LENGTH
#define BOND_SAVE_MAX_DB_SIZE               (NVDS_TAG_BOND_GATT_DB_END - NVDS_TAG_BOND_GATT_DB_START + 1)
//NVDS LEN
#define NVDS_LEN_BOND_SAVE_SECURITY         sizeof(bond_security_t)
#define NVDS_LEN_BOND_SAVE_SIGN_COUNTER     sizeof(sign_counter_t)
#define NVDS_LEN_BOND_SAVE_DATABASE_IDX     sizeof(bond_database_idx_store_t)
#define NVDS_LEN_BOND_SAVE_CCCD             sizeof(bond_cccd_store_t)
#define NVDS_LEN_BOND_SAVE_AGE              BOND_SAVE_MAX_DEVICE_CNT
#define NVDS_LEN_BOND_SAVE_DB_USAGE         ( (BOND_SAVE_MAX_DB_SIZE/8) + ((BOND_SAVE_MAX_DB_SIZE%8)?1:0) )   

/*
 * MACRO
 ****************************************************************************************
 */
#define BOND_SAVE_DEVICE_TO_TAG_SECURITY(device)        (NVDS_TAG_BOND_DEV_INFO_START + device * 4 + 0)
#define BOND_SAVE_DEVICE_TO_TAG_SIGNCNT(device)         (NVDS_TAG_BOND_DEV_INFO_START + device * 4 + 1)
#define BOND_SAVE_DEVICE_TO_TAG_DATABASE(device)        (NVDS_TAG_BOND_DEV_INFO_START + device * 4 + 2)
#define BOND_SAVE_DEVICE_TO_TAG_CCCD(device)            (NVDS_TAG_BOND_DEV_INFO_START + device * 4 + 3)



/*
 * FUNCTION DECLEAR
 ****************************************************************************************
 */
//basic set get
ble_bond_err_t bond_save_read (bond_node_id_t index , bond_type_t type , void *buff);
ble_bond_err_t bond_save_write(bond_node_id_t index , bond_type_t type , void* buff);
//services
ble_bond_err_t bond_save_read_services (uint8_t tag , uint8_t length , uint8_t *buff);
ble_bond_err_t bond_save_write_services_and_usage (uint8_t tag , uint8_t length , uint8_t *buff , bool add_usage);
ble_bond_err_t bond_save_delete_services_and_usage(uint8_t tag);
ble_bond_err_t bond_save_delete_multi_services_and_usage(bond_database_idx_store_t *db);
//age
uint8_t *get_bond_save_age(void);
void set_bond_save_age    (void);
//db usage
void bond_save_db_usage_get(void);
void bond_save_db_usage_set(void);
uint8_t *get_bound_save_db_usage_buff(void);
//tags
ble_bond_err_t bond_save_delete_tag(uint8_t tag);



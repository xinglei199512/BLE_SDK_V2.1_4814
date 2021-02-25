/**
 ****************************************************************************************
 *
 * @file   bond_save_db_cccd.h
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



/*
 * FUNCTION DECLEAR
 ****************************************************************************************
 */
//database 
ble_bond_err_t bond_save_set_database(bond_node_id_t id , bond_database_t *db);
ble_bond_err_t bond_save_find_service_tag_from_id_uuid(bond_node_id_t id , uint8_t *uuid , uint8_t *tag);
//cccd
ble_bond_err_t bond_save_set_cccd(bond_node_id_t id , bond_cccd_t *cccd);
ble_bond_err_t bond_save_get_cccd_from_id_handle(uint8_t id , uint16_t attr_handle , uint16_t *value);

//cache
void set_cccd_cache_invalid(void);

